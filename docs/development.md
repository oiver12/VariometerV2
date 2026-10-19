# Developer guide

[Project overview](../README.md)

Commands below assume a Linux shell. Start from the repository root unless a command changes directories.

## Clone

```sh
git clone --recurse-submodules https://github.com/oiver12/VariometerV2.git
cd VariometerV2
```

For an existing clone, run `git submodule update --init --recursive` from the root. The primary firmware is in `Software/variometer-software/`. The archived FreeRTOS runner retains a separate checkout at `Simulation/Main/variometer-software/`, pinned to its historical firmware version.

## Build & flash

Use **ESP-IDF v5.1.4** with the **ESP32-S3** target. The SDK version is pinned in `main/idf_component.yml` and `dependencies.lock`.

For a new SDK installation (outside this repository):

```sh
git clone --branch v5.1.4 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32s3
. ./export.sh
```

Return to the project checkout, then build and flash:

```sh
cd Software/variometer-software
idf.py build
# Substitute your device's serial port.
idf.py -p /dev/ttyUSB0 flash monitor
```

For later shell sessions, activate the SDK with `. /path/to/esp-idf/export.sh` before building. The checked-in `sdkconfig` already selects ESP32-S3; review `main/pinDefines.h` and the PCB revision before flashing. See [Espressif’s v5.1.4 setup guide](https://docs.espressif.com/projects/esp-idf/en/v5.1.4/esp32s3/get-started/index.html).

## Simulation & replay

### Offline sensor replay

From the repository root, with Python and GCC available:

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install -r VariometerAnalysis/requirements.txt
python VariometerAnalysis/replay_flight.py \
  Simulation/Main/VarioLogs/vario2.txt \
  --dt 0.03 --allow-truncated-tail \
  --output docs/images/flight-replay.png
```

The script compiles the firmware’s existing quaternion and Kalman functions into a temporary host library and feeds in recorded acceleration, angular velocity, magnetic field, and pressure. The upper panel compares relative barometric altitude with the Kalman estimate; the lower compares an unsmoothed barometric difference with estimated climb rate.

The raw log has **no timestamps**. `--dt` is an explicit assumption; the horizontal axis is sample index. The script uses the firmware’s fixed-temperature pressure conversion, discards an incomplete final record only when requested, and writes a JSON file recording its assumptions and input hash. It does not reproduce task scheduling, startup warm-up, or sensor calibration, and is not a latency benchmark.

### FreeRTOS sensor mocks

With `ISSIMULATION` defined, hardware reads are replaced by `BMI088_Simulation.c`, `QMC588L_Simulation.c`, and the historically named `DPS310_SImulation.c`. These call `Simulation/Main/simulationData.c`, which supplies sensor samples and GPS updates from `VarioLogs/`. ESP-IDF mocks provide host interfaces; the bundled FreeRTOS POSIX port schedules application tasks.

On Debian/Ubuntu, install GCC, Make, SDL2 headers, and pkg-config (`build-essential`, `libsdl2-dev`, `pkg-config`). Then:

```sh
cd Simulation/Main
make clean
make -j2 SANITIZE_ADDRESS=1 USER_DEMO=FULL_DEMO
ASAN_OPTIONS=detect_leaks=0 ./build/posix_demo
```

The sanitizer build is the validated host replay configuration; the archived optimized build remains unstable. The runner defaults to `VarioLogs/vario2.txt` and `VarioLogs/gps2.txt`, warns about incomplete final records, and exits at the end of the sensor recording. Select another pair with:

```sh
VARIO_REPLAY_VARIO=VarioLogs/vario.txt \
VARIO_REPLAY_GPS=VarioLogs/gps.txt \
ASAN_OPTIONS=detect_leaks=0 ./build/posix_demo
```

Host timing is not an embedded performance measurement. The archived mocks do not implement every hardware service.

### Screen simulator

```sh
cd Screen_Simulator_Linux
make
make run
```

Use up/down arrows to change screens, `m` for the menu/power button, Enter to select, `p` for a popup, and `f` to advance replay. The IGC recording is selected in `main.c`.

Regenerate the screen captures from the repository root after building:

```sh
python tools/capture_screens.py
```

The capture helper requires Pillow, included in `VariometerAnalysis/requirements.txt`.

## Firmware source map

| Path | Purpose |
| --- | --- |
| `Software/variometer-software/main/Sensor/` | Sensor drivers and simulation implementations |
| `main/QuaternionFilter.c`, `main/Kalman/` | Orientation and altitude/climb-rate estimation |
| `main/DeviceContext.*`, `main/FlightState.*` | Shared state and flight-state logic |
| `main/Screen/` | Render system, screens, widgets, e-ink integration |
| `main/IGC/`, `main/nmea_parser.*` | Flight logging and GPS parsing |
| `main/AirspaceParcer/`, `main/Bluetooth/`, `main/Watchdog/` | Airspace, BLE, watchdog |
| `Simulation/Main/` | FreeRTOS host runner and ESP-IDF mocks |
| `Screen_Simulator_Linux/`, `Screen_Simulator/` | Linux UI simulator and earlier Windows version |
| `VariometerAnalysis/` | Analysis and offline replay |
| `Hardware/`, `Case/`, `Documentation/` | Electronics, enclosure, design notes |
| `Website/` | Supporting web experiments |

Paths beginning with `main/` are relative to `Software/variometer-software/`.

`main/MainTasks.c` defines the measurement and filter task entry points. BLE is experimental; `startBluetooth()` is disabled in the checked-in startup configuration.

## Regenerating hardware visuals

Regenerate the PCB and schematic SVGs and angled 3D views from the repository root:

```sh
python3 tools/export_kicad.py
```

The exports and angled renders were generated with **KiCad 9.0.9**. Install KiCad’s standard 3D models as well; `--models /path/to/3dmodels` selects a custom location. The script resolves old model paths in temporary copies and records unresolved references in `docs/images/pcb-render-manifest.json`. KiCad 7/8 can export SVGs; `pcb render` requires KiCad 9+. See the [KiCad CLI reference](https://docs.kicad.org/9.0/en/cli/cli.html).

