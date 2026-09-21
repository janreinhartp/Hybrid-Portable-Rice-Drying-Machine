#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define HMI_SESSION_ID_LENGTH 24
#define HMI_FAULT_DESCRIPTION_LENGTH 96
#define HMI_INTERLOCK_REASON_LENGTH 96
#define HMI_MAX_ACTIVE_FAULTS 16

typedef enum {
    HMI_MACHINE_IDLE = 0,
    HMI_MACHINE_DRYING,
    HMI_MACHINE_PAUSED,
    HMI_MACHINE_COMPLETE,
    HMI_MACHINE_FAULT,
    HMI_MACHINE_EMERGENCY_STOP,
} hmi_machine_state_t;

typedef enum {
    HMI_ACTUATOR_OFF = 0,
    HMI_ACTUATOR_ON,
    HMI_ACTUATOR_FAULT,
    HMI_ACTUATOR_INTERLOCKED,
} hmi_actuator_state_t;

typedef enum {
    HMI_ACTUATOR_ELEVATOR = 0,
    HMI_ACTUATOR_FAN,
    HMI_ACTUATOR_HEATER,
    HMI_ACTUATOR_DISCHARGE,
} hmi_actuator_t;

typedef enum {
    HMI_FAULT_INFO = 0,
    HMI_FAULT_WARNING,
    HMI_FAULT_CRITICAL,
} hmi_fault_severity_t;

typedef struct {
    uint32_t id;
    hmi_fault_severity_t severity;
    bool active;
    time_t timestamp;
    char description[HMI_FAULT_DESCRIPTION_LENGTH];
} hmi_fault_t;

typedef struct {
    hmi_actuator_state_t requested;
    hmi_actuator_state_t actual;
    char reason[HMI_INTERLOCK_REASON_LENGTH];
} hmi_actuator_status_t;

typedef struct {
    float temperature_setpoint_c;
    float maximum_temperature_c;
    float temperature_hysteresis_c;
    float target_moisture_percent;
    uint32_t moisture_confirmation_seconds;
    uint32_t drying_timeout_seconds;
    uint32_t discharge_seconds;
    uint32_t logging_interval_seconds;
} hmi_settings_t;

typedef struct {
    time_t current_datetime;
    bool rtc_valid;
    hmi_machine_state_t machine_state;
    float upper_temperature_c;
    float upper_humidity_percent;
    float lower_temperature_c;
    float lower_humidity_percent;
    float temperature_difference_c;
    float moisture_raw;
    float moisture_filtered;
    float target_moisture_percent;
    float temperature_setpoint_c;
    hmi_actuator_status_t elevator;
    hmi_actuator_status_t fan;
    hmi_actuator_status_t heater;
    hmi_actuator_status_t discharge;
    uint32_t drying_elapsed_seconds;
    uint32_t heater_runtime_seconds;
    uint32_t fan_runtime_seconds;
    char session_id[HMI_SESSION_ID_LENGTH];
    hmi_fault_t active_faults[HMI_MAX_ACTIVE_FAULTS];
    size_t active_fault_count;
    bool sd_available;
    uint64_t sd_free_bytes;
} hmi_state_t;
