#!/usr/bin/env python3
"""Capture actual Linux simulator screens as PNGs (requires Pillow)."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path,
                        default=ROOT / "Screen_Simulator_Linux/build/screen-simulator")
    parser.add_argument("--output", type=Path, default=ROOT / "docs/images")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="variometer-screens-") as temporary:
        subprocess.run([str(args.binary.resolve()), "--capture-dir", temporary],
                       cwd=ROOT / "Screen_Simulator_Linux",
                       env=dict(os.environ, SDL_VIDEODRIVER="dummy"),
                       timeout=30, check=True)
        for name in ("screen-flight", "screen-navigation", "screen-settings"):
            with Image.open(Path(temporary) / f"{name}.bmp") as frame:
                frame.convert("RGB").save(args.output / f"{name}.png")
            print(args.output / f"{name}.png")


if __name__ == "__main__":
    main()
