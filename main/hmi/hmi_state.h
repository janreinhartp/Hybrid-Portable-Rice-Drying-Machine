#pragma once

#include "esp_err.h"
#include "hmi_types.h"

esp_err_t hmi_state_init(void);
esp_err_t hmi_state_read(hmi_state_t *state);
esp_err_t hmi_state_publish(const hmi_state_t *state);
