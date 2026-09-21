#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "hmi_types.h"

bool hmi_rtc_is_valid_datetime(int64_t timestamp_seconds);
esp_err_t hmi_rtc_apply_state(hmi_state_t *state, int64_t timestamp_seconds, bool rtc_valid);
