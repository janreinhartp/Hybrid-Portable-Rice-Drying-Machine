# Rice Dryer HMI Implementation Memory

## Project boundary

- The ESP-IDF project is at the repository root.
- `Documentation/Prompt.txt` is the implementation requirement source.
- `Documentation/16_LVGL_UI` is read-only reference material. It is not copied, linked, modified, or included in the root build.

## Completed

### Phase 0: Project scaffold

- Created the root ESP-IDF project with `CMakeLists.txt`, `main/`, `sdkconfig.defaults`, and `partitions.csv`.
- Added independent source boundaries:
  - `main/board`: board and LVGL ownership boundary
  - `main/hmi`: state, commands, navigation, and screens
  - `main/machine`: controller and safety-facing logic
  - `main/storage`: reserved for NVS, SD, logging, calibration, and history
  - `main/diagnostics`: reserved for health and resource reporting
- Added the LVGL 8.x managed component dependency.

### Phase 1: Build configuration

- Standardized on ESP-IDF 5.5.4.
- Target is `esp32s3`.
- Configured 16 MB flash and the custom 3 MB factory partition in `partitions.csv`.
- Configured PSRAM and 16-bit LVGL color depth.
- Installed the required ESP-IDF Python environment and build tools locally.
- Verified the project builds and links with the ESP32-S3 toolchain.

Build environment used:

```powershell
$env:Path = 'C:\Espressif\tools\cmake\3.30.2\bin;C:\Espressif\tools\ninja\1.12.1;C:\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20260121\xtensa-esp-elf\bin;' + $env:Path
$env:ESP_ROM_ELF_DIR = 'C:\Espressif\tools\esp-rom-elfs\20241011'
& 'C:\Users\janre\.espressif\python_env\idf5.5_py3.13_env\Scripts\python.exe' 'C:\esp\v5.5.4\esp-idf\tools\idf.py' build
```

Latest build result:

- ESP-IDF: 5.5.4
- Target: ESP32-S3
- LVGL: 8.4.0
- Firmware image: 262,512 bytes
- Factory app partition: 3 MB
- App partition free: 92%
- The project has not been flashed to hardware yet.

### Phase 2: HMI and controller contracts

- Added `hmi_state_t` with machine state, sensor values, actuator requested/actual state, runtimes, session ID, faults, RTC, and SD fields.
- Added a mutex-protected HMI state snapshot API.
- Added an HMI command queue for drying, actuator, settings, RTC, and calibration commands.
- Added a machine-controller FreeRTOS task that consumes commands.
- Emergency-stop commands force all actuator states off and set the machine state to emergency stop.
- Actuator-on requests remain explicitly `INTERLOCKED` because no physical actuator driver or safety service is connected yet.
- Actuator-off requests publish an explicit controller-owned off state.
- No UI code directly calls GPIO, relay, Modbus, sensor, or storage code.

### Phase 3: Board and LVGL display runtime

- Implemented an independent RGB LCD port under `main/board/board_port.c`.
- Configured the verified reference display facts: 1024x600 RGB565, 30.85 MHz pixel clock, 16-bit RGB bus, and the Waveshare GPIO map.
- Allocated two RGB framebuffers in PSRAM.
- Added the LVGL display flush callback and framebuffer registration.
- Added the LVGL 2 ms tick timer and dedicated LVGL task.
- Added recursive LVGL locking APIs and used the lock during screen creation.
- The display runtime builds successfully; it has not yet been flashed to hardware.

### Phase 4: First HMI screen

- Replaced the placeholder label with a Home screen driven only by `hmi_state_t`.
- Added 250 ms LVGL-owned refreshes for RTC validity, machine state, upper/lower chamber values, moisture target, actual actuator states, and SD availability.
- Added safe label-allocation handling.
- Latest image after the Home screen: 454,544 bytes, with 86% of the 3 MB factory app partition free.

### Phase 5: Touch and navigation runtime

- Implemented an independent GT911 I2C input path under `main/board/touch_input.c`.
- Configured the GT911 address at `0x5D`, I2C on GPIO8/GPIO9, and touch interrupt input on GPIO4.
- Added LVGL pointer input registration with coordinate clamping to 1024x600.
- Added a central LVGL screen manager with Home, Drying, and Manual screens.
- Added navigation buttons between the three screens.
- Added Manual ON/OFF controls that send `HMI_COMMAND_SET_ACTUATOR` commands through the HMI queue.
- The complete display, touch, navigation, and controller path builds successfully.
- Latest image: 487,424 bytes, with 85% of the 3 MB factory app partition free.

## Current limitations

- Physical validation is still required for RGB timing, GT911 address/orientation, and touch coordinates.
- Backlight control and board reset handling are not implemented yet.
- Settings, Calibration, RTC, Alarms, History, SD Status, and System Diagnostics screens are not implemented yet.
- Sensor, actuator, safety, RTC, SD, NVS, calibration, history, and diagnostics implementations are not connected.
- The controller intentionally refuses to claim successful actuator-on operations.
- No hardware flash, monitor session, touch verification, or runtime memory measurement has been completed.

## Next phases

### Phase 6: Hardware validation and board polish

1. Verify the Waveshare ESP32-S3-Touch-LCD-7B schematic for RGB timing, GPIO mapping, GT911 pins/address, orientation, backlight, and I2C details.
2. Flash the current firmware and verify display output, touch press/release, and coordinate orientation.
3. Add backlight and board reset handling after hardware verification.
4. Add a physical emergency-stop input boundary before enabling real actuator outputs.

### Phase 7: HMI foundation

1. Add the central screen manager and shared industrial theme.
2. Add reusable status, sensor, actuator, alarm, moisture, and runtime widgets.
3. Implement the Home screen from `hmi_state_t` only.
4. Add the 100-250 ms UI refresh mechanism owned by the LVGL task.
5. Verify that non-LVGL tasks never manipulate LVGL objects.

### Phase 8: Machine services

1. Add sensor manager interfaces and validated sensor fault states.
2. Add actuator manager interfaces behind the controller and safety manager.
3. Add emergency-stop input handling with physical safety authority.
4. Add settings validation and NVS persistence.
5. Add RTC read/set/verify behavior and timestamp propagation.
6. Add SD logging, bounded history queries, calibration storage, and failure strategy.

### Phase 9: Remaining screens

Implement in this order:

1. Drying
2. Manual Control
3. Settings
4. Moisture Calibration
5. RTC
6. Alarms
7. Drying History
8. SD Card Status
9. System Diagnostics

Each screen must consume HMI state and send HMI commands. It must not own machine logic or access concrete hardware drivers.

### Phase 10: Verification and optimization

- Add tests for settings bounds, command safety rejection, moisture progress edge cases, RTC validity, calibration persistence, alarm timestamps, and bounded history queries.
- Build and flash with ESP-IDF 5.5.4.
- Verify large touch targets, orientation, numeric input, navigation, alarms, requested-versus-actual states, and emergency-stop behavior.
- Measure heap, PSRAM, task stacks, LVGL refresh behavior, and history browsing.
- Re-run `idf.py size` after each substantial UI or driver phase.
