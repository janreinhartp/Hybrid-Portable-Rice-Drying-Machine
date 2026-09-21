#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "esp_err.h"
#include "hmi_types.h"

void hmi_settings_init(hmi_settings_t *settings);
bool hmi_settings_validate(const hmi_settings_t *settings, char *error, size_t error_size);
esp_err_t hmi_settings_apply(hmi_state_t *state, const hmi_settings_t *settings);
