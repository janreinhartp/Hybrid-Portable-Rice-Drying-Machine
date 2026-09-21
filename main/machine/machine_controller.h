#pragma once

#include "esp_err.h"
#include "hmi_command.h"

esp_err_t machine_controller_init(void);
esp_err_t machine_controller_start(void);
esp_err_t machine_controller_process(const hmi_command_t *command);
