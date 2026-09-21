# Rice Dryer HMI Firmware

Independent ESP-IDF firmware for the Waveshare ESP32-S3-Touch-LCD-7B rice dryer HMI.

`Documentation/Prompt.txt` is the implementation requirements document. `Documentation/16_LVGL_UI` is read-only reference material and is not copied, linked, modified, or included in this build.

## Current implementation

- ESP-IDF 5.5.4 targeting ESP32-S3
- LVGL 8.4.0 managed component
- 1024x600 RGB565 display configuration with PSRAM framebuffers
- Dedicated LVGL task, 2 ms tick, display flush callback, and recursive LVGL lock
- Independent GT911 I2C touch input on GPIO8/GPIO9 with pointer registration
- Thread-safe `hmi_state_t` snapshots
- HMI command queue and machine-controller task
- Emergency-stop state handling
- Explicit requested-versus-actual actuator states
- Actuator interlock reporting while physical drivers are disconnected
- Central navigation for Home, Drying, and Manual screens
- Home screen state refresh at 250 ms
- Manual actuator controls that send HMI commands instead of touching hardware
- Custom 16 MB flash configuration and 3 MB factory app partition

## Project structure

- `main/board`: RGB display, LVGL runtime, and GT911 input boundary
- `main/hmi`: state, commands, screen manager, and screen UI
- `main/machine`: controller and safety-facing interfaces
- `main/storage`: reserved for NVS, SD, logging, calibration, and history
- `main/diagnostics`: reserved for system health and resource reporting
- `Documentation`: requirements, schematic, and reference-only material
- `memory.md`: implementation history, validation results, limitations, and next phases

## Build

The project uses ESP-IDF 5.5.4 at `C:\esp\v5.5.4\esp-idf`. The VS Code workspace is configured to use that installation.

From the repository root in an activated ESP-IDF 5.5.4 shell:

```powershell
idf.py set-target esp32s3
idf.py build
idf.py size
```

The current validated build uses the ESP32-S3 compiler, CMake 3.30.2, Ninja 1.12.1, and the ESP-IDF ROM ELF symbols. The firmware has not yet been flashed to hardware.

## Current limitations

The display, touch, and navigation code has compiled successfully but still requires physical-board validation. Backlight/reset control, real sensors, actuator drivers, safety input, RTC, NVS, SD logging, calibration, history, diagnostics, and the remaining screens are not connected yet. Actuator-on commands remain interlocked and never claim successful physical output.

See [memory.md](memory.md) for the complete phased implementation record and next steps.
