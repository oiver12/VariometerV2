#!/usr/bin/env python3
"""Replay raw sensor records through the firmware's quaternion and Kalman code.

Format: ten little-endian float32s (acc[3], gyro[3], mag[3], pressure), then LF.
These recordings have no timestamps. --dt is an explicit modelling assumption,
not a recovered measurement interval. This is an offline filter replay, not a
replica of FreeRTOS task scheduling, buffering, calibration, or startup warm-up.
No GPS coordinates or pilot identities are read or included in the plot.
"""
import argparse
import ctypes
import hashlib
import json
import math
from pathlib import Path
import struct
import subprocess
import tempfile
import warnings

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

ROOT = Path(__file__).resolve().parents[1]


def read_records(path, allow_truncated_tail):
    raw = path.read_bytes()
    count, tail = divmod(len(raw), 41)
    if tail:
        message = f"{path.name}: incomplete final record ({tail} bytes)"
        if not allow_truncated_tail:
            raise ValueError(message + "; use --allow-truncated-tail to discard it")
        warnings.warn(message + "; discarded", stacklevel=2)
    if count < 2:
        raise ValueError("Need at least two complete records")
    records = []
    for i in range(count):
        offset = 41 * i
        if raw[offset + 40] != 10:
            raise ValueError(f"Invalid record delimiter at record {i}")
        values = struct.unpack_from("<10f", raw, offset)
        if not all(math.isfinite(x) for x in values) or values[9] <= 0:
            raise ValueError(f"Invalid sensor value at record {i}")
        records.append(values)
    return np.asarray(records, dtype=np.float32), tail, hashlib.sha256(raw).hexdigest()


def replay(records, altitude, dt, firmware):
    main = firmware / "main"
    with tempfile.TemporaryDirectory(prefix="variometer-filter-") as temporary:
        library = Path(temporary) / "replay.so"
        subprocess.run(["gcc", "-shared", "-fPIC", "-O2", "-I", str(main),
                        str(Path(__file__).with_name("replay_filter.c")),
                        str(main / "QuaternionFilter.c"),
                        *(str(main / "Kalman" / name) for name in
                          ("kalman.c", "matrix.c", "cholesky.c")),
                        "-lm", "-o", str(library)], check=True)
        filters = ctypes.CDLL(str(library))
        float_pointer = ctypes.POINTER(ctypes.c_float)
        filters.replay_step.argtypes = [float_pointer, ctypes.c_float,
                                       ctypes.c_float, float_pointer]
        filters.replay_step.restype = None
        filters.replay_init.restype = None
        filters.replay_init()
        result = np.empty((len(records), 2), dtype=np.float32)
        for i, record in enumerate(records):
            filters.replay_step(record.ctypes.data_as(float_pointer), altitude[i], dt,
                                result[i].ctypes.data_as(float_pointer))
        if not np.isfinite(result).all():
            raise ValueError("Filter produced a non-finite estimate")
        return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("recording", type=Path)
    parser.add_argument("--dt", type=float, required=True,
                        help="Assumed sample interval in seconds; logs have no timestamps")
    parser.add_argument("--allow-truncated-tail", action="store_true")
    parser.add_argument("--firmware", type=Path,
                        default=ROOT / "Software/variometer-software")
    parser.add_argument("--output", type=Path,
                        default=ROOT / "docs/images/flight-replay.png")
    args = parser.parse_args()
    if not math.isfinite(args.dt) or args.dt <= 0:
        parser.error("--dt must be finite and positive")
    try:
        records, discarded, digest = read_records(args.recording, args.allow_truncated_tail)
        # Same fixed-temperature pressure-to-height expression as MainTasks.c,
        # referenced to the mean of the first ten pressures available in this log.
        pressure = records[:, 9].astype(float)
        reference = pressure[:10].mean()
        altitude = 293.15 / 0.0065 * ((pressure / reference) **
                                    (-0.0065 * 287.053 / 9.80665) - 1)
        estimate = replay(records, altitude, args.dt, args.firmware)
    except (ValueError, OSError) as error:
        parser.error(str(error))
    samples = np.arange(len(records))
    fig, axes = plt.subplots(2, 1, figsize=(11, 7), sharex=True, constrained_layout=True)
    axes[0].plot(samples, altitude, color="#94a3b8", lw=1, label="Raw barometric altitude")
    axes[0].plot(samples, estimate[:, 0], color="#007f86", lw=1.5, label="Kalman altitude")
    axes[0].set_ylabel("Relative altitude (m)")
    axes[1].plot(samples[1:], np.diff(altitude) / args.dt, color="#94a3b8", lw=0.8,
                 label="Raw barometric difference / assumed dt")
    axes[1].plot(samples, estimate[:, 1], color="#007f86", lw=1.5,
                 label="Kalman climb rate")
    axes[1].set_ylabel("Climb rate (m/s)")
    axes[1].set_xlabel("Recorded sample index (no timestamps in source)")
    for axis in axes:
        axis.grid(alpha=0.2)
        axis.legend(loc="upper left", frameon=False)
    fig.suptitle(f"Offline sensor replay · {args.recording.name}\n"
                 f"Assumed dt = {args.dt:g} s; fixed-temperature model; no latency benchmark")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(args.output, dpi=160)
    plt.close(fig)
    metadata = {"source": args.recording.name, "source_sha256": digest,
                "complete_records": len(records), "discarded_tail_bytes": discarded,
                "assumed_dt_seconds": args.dt, "timestamps_available": False,
                "method": "Firmware filter functions with one sensor update per record; "
                          "not a scheduler replay or latency benchmark"}
    args.output.with_suffix(".json").write_text(json.dumps(metadata, indent=2) + "\n")
    print(args.output)


if __name__ == "__main__":
    main()
