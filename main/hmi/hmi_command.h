#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "hmi_types.h"

typedef enum {
    HMI_COMMAND_START_DRYING = 0,
    HMI_COMMAND_STOP_DRYING,
    HMI_COMMAND_EMERGENCY_STOP,
    HMI_COMMAND_SET_ACTUATOR,
    HMI_COMMAND_SAVE_SETTINGS,
    HMI_COMMAND_SET_RTC,
    HMI_COMMAND_SAVE_CALIBRATION,
} hmi_command_type_t;

typedef struct {
    hmi_command_type_t type;
    union {
        struct {
            hmi_actuator_t actuator;
            bool on;
        } actuator;
        hmi_settings_t settings;
        struct {
            int64_t datetime;
        } rtc;
    } payload;
} hmi_command_t;

esp_err_t hmi_command_init(size_t queue_length);
esp_err_t hmi_command_send(const hmi_command_t *command, uint32_t timeout_ms);
esp_err_t hmi_command_receive(hmi_command_t *command, uint32_t timeout_ms);
