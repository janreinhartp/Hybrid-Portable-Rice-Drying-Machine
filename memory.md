# Rice Dryer HMI Implementation Memory

## Project boundary

- The ESP-IDF project is rooted at the repository root.
- The implementation targets the requirements in [Documentation/Prompt.txt](Documentation/Prompt.txt).
- The material in [Documentation/16_LVGL_UI](Documentation/16_LVGL_UI) is reference-only and is not copied into the build.
- The code is organized around the architecture requested by the prompt:
  - `main/board`: display, LVGL runtime, and touch boundary
  - `main/hmi`: HMI state, commands, settings, calibration, RTC, fault, history, diagnostics, and screen manager
  - `main/machine`: controller and safety-facing state logic
  - `main/storage`: reserved for persistent storage and future RTC/SD work
  - `main/diagnostics`: reserved for health and resource reporting

## Verified implementation facts

### Hardware wiring reference

- The project currently includes a design-level wiring concept for the system architecture, but the exact final pin mapping should be confirmed with the Waveshare board schematic and the physical electronics revision before use in production.
- The conceptual connection strategy is intended to document the logical device map: MCU to LCD, touch controller, RTC, SD, sensors, actuator drivers, and safety interlocks.
- The actual implementation remains safety-interlocked and does not claim real hardware output until the final board wiring and actuator drivers are validated.

### Build status

- Verified build command: `idf.py build`
- Result: successful build in the current workspace
- Generated binary: `build/rice_dryer_hmi.bin`
- Evidence from the terminal: “Project build complete.”

### Platform and runtime

- ESP-IDF version: 5.5.4
- Target: ESP32-S3
- LVGL version: 8.4.0 via managed component
- Display: 1024x600 RGB565
- Touch controller: GT911 on the Waveshare board path
- PSRAM: enabled for LVGL framebuffers

## Current requirement coverage

### Requirement: LVGL-based HMI separated from machine control

Status: implemented.

- The UI runtime is isolated in the board and HMI layers.
- Machine logic lives behind the controller boundary and is not directly controlled from LVGL code.
- HMI state is shared through a clear state contract rather than direct hardware calls.

### Requirement: Home, Drying, and Manual screens

Status: implemented in the current screen manager.

- Home, Drying, and Manual screens are part of the active navigation flow.
- The shared HMI state is refreshed at a regular interval for the current screen content.
- Manual actuator commands are routed through HMI commands instead of direct GPIO/relay calls.

### Requirement: Settings, calibration, RTC, alarms, history, diagnostics models

Status: implemented at the data-model and logic level.

- Settings validation and defaults are implemented in [main/hmi/hmi_settings.c](main/hmi/hmi_settings.c).
- Calibration point storage and interpolation are implemented in [main/hmi/hmi_calibration.c](main/hmi/hmi_calibration.c).
- RTC validation and state application helpers are implemented in [main/hmi/hmi_rtc.c](main/hmi/hmi_rtc.c).
- Fault tracking and active alarm handling are implemented in [main/hmi/hmi_faults.c](main/hmi/hmi_faults.c).
- Bounded drying history storage exists in [main/hmi/hmi_history.c](main/hmi/hmi_history.c).
- Diagnostics summary generation exists in [main/hmi/hmi_diagnostics.c](main/hmi/hmi_diagnostics.c).
- These modules are registered in [main/CMakeLists.txt](main/CMakeLists.txt).

### Requirement: full LVGL screen workflows for the remaining prompt sections

Status: still pending as completed UI screens.

- The underlying logic modules exist, but the prompt’s dedicated Settings, Calibration, RTC, Alarm, History, and Diagnostics screens are not yet fully implemented as end-to-end LVGL interactions.
- The project has a working architecture and contract layer, but the remaining screens still represent the next phase of UI work.

### Requirement: real hardware integration

Status: intentionally blocked until the board is validated.

- Real sensors, relays, SSRs, RTC persistence, SD logging, and board-level safety wiring are not connected yet.
- The machine controller intentionally keeps actuator output claims interlocked until actual driver validation is available.

## Current project status

### Completed

- Project scaffold and source partitioning
- ESP-IDF + LVGL display/touch runtime
- HMI command/state model
- Machine controller boundary
- Settings and validation support
- Calibration contract and interpolation support
- RTC validation support
- Fault model and warning/critical handling
- History storage support
- Diagnostics summary support
- Home/Drying/Manual screen flow
- Successful project build verification

### Still pending

- Full dedicated LVGL screens for Settings, Calibration, RTC, Alarms, History, and Diagnostics
- Physical board validation on the actual Waveshare ESP32-S3 panel
- Real I/O wiring for sensors, storage, and actuator controllers
- RTC/NVS/SD persistence and real event logging
- Safety integration and actuator enablement on hardware

## Recommended next implementation order

1. Build the remaining prompt-screen UI flows on top of the existing shared state contract
2. Add calibration-screen persistence and user validation flows
3. Add RTC-screen configuration and validation flow
4. Add alarm/history/diagnostics LVGL screens
5. Validate hardware behavior on the actual board
6. Integrate real sensors and actuator drivers
7. Confirm safety behavior and operational mode transitions on hardware

## Final assessment

The firmware has moved past the initial skeleton and now includes a verified buildable ESP-IDF + LVGL foundation with the requested HMI/controller separation and the core prompt-related data models. The remaining effort is primarily in completing the dedicated LVGL screen implementations and validating the system against the real hardware, not in reworking the project architecture.
