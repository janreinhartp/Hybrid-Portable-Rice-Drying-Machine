#include "hmi_rtc.h"

#include <time.h>

bool hmi_rtc_is_valid_datetime(int64_t timestamp_seconds)
{
    if (timestamp_seconds <= 0) {
        return false;
    }

    struct tm tm_value;
    time_t raw_time = (time_t)timestamp_seconds;

    if (gmtime_r(&raw_time, &tm_value) == NULL) {
        return false;
    }

    return true;
}

esp_err_t hmi_rtc_apply_state(hmi_state_t *state, int64_t timestamp_seconds, bool rtc_valid)
{
    if (state == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (rtc_valid && !hmi_rtc_is_valid_datetime(timestamp_seconds)) {
        return ESP_ERR_INVALID_STATE;
    }

    state->current_datetime = (time_t)timestamp_seconds;
    state->rtc_valid = rtc_valid;
    return ESP_OK;
}
