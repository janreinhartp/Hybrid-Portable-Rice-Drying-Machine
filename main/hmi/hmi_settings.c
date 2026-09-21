#include "hmi_settings.h"

#include <stdio.h>
#include <string.h>

void hmi_settings_init(hmi_settings_t *settings)
{
    if (settings == NULL) {
        return;
    }

    memset(settings, 0, sizeof(*settings));
    settings->temperature_setpoint_c = 50.0f;
    settings->maximum_temperature_c = 70.0f;
    settings->temperature_hysteresis_c = 2.0f;
    settings->target_moisture_percent = 14.0f;
    settings->moisture_confirmation_seconds = 30;
    settings->drying_timeout_seconds = 3600;
    settings->discharge_seconds = 30;
    settings->logging_interval_seconds = 10;
}

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error == NULL || error_size == 0) {
        return;
    }
    snprintf(error, error_size, "%s", message);
}

bool hmi_settings_validate(const hmi_settings_t *settings, char *error, size_t error_size)
{
    if (settings == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Settings pointer is NULL");
        }
        return false;
    }

    if (error != NULL && error_size > 0) {
        error[0] = '\0';
    }

    if (settings->temperature_setpoint_c < 20.0f || settings->temperature_setpoint_c > 90.0f) {
        set_error(error, error_size, "Temperature setpoint must be between 20.0 C and 90.0 C");
        return false;
    }
    if (settings->maximum_temperature_c < 25.0f || settings->maximum_temperature_c > 120.0f) {
        set_error(error, error_size, "Maximum temperature must be between 25.0 C and 120.0 C");
        return false;
    }
    if (settings->temperature_hysteresis_c < 0.5f || settings->temperature_hysteresis_c > 20.0f) {
        set_error(error, error_size, "Temperature hysteresis must be between 0.5 C and 20.0 C");
        return false;
    }
    if (settings->target_moisture_percent < 5.0f || settings->target_moisture_percent > 30.0f) {
        set_error(error, error_size, "Target moisture must be between 5.0 %% and 30.0 %%");
        return false;
    }
    if (settings->moisture_confirmation_seconds < 5 || settings->moisture_confirmation_seconds > 600) {
        set_error(error, error_size, "Moisture confirmation time must be between 5 s and 600 s");
        return false;
    }
    if (settings->drying_timeout_seconds < 300 || settings->drying_timeout_seconds > 86400) {
        set_error(error, error_size, "Drying timeout must be between 300 s and 86400 s");
        return false;
    }
    if (settings->discharge_seconds < 5 || settings->discharge_seconds > 600) {
        set_error(error, error_size, "Discharge time must be between 5 s and 600 s");
        return false;
    }
    if (settings->logging_interval_seconds < 5 || settings->logging_interval_seconds > 3600) {
        set_error(error, error_size, "Logging interval must be between 5 s and 3600 s");
        return false;
    }

    if (settings->maximum_temperature_c <= settings->temperature_setpoint_c) {
        set_error(error, error_size, "Maximum temperature must be greater than the temperature setpoint");
        return false;
    }

    return true;
}

esp_err_t hmi_settings_apply(hmi_state_t *state, const hmi_settings_t *settings)
{
    if (state == NULL || settings == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!hmi_settings_validate(settings, NULL, 0)) {
        return ESP_ERR_INVALID_STATE;
    }

    state->temperature_setpoint_c = settings->temperature_setpoint_c;
    state->maximum_temperature_c = settings->maximum_temperature_c;
    state->temperature_hysteresis_c = settings->temperature_hysteresis_c;
    state->target_moisture_percent = settings->target_moisture_percent;
    state->moisture_confirmation_seconds = settings->moisture_confirmation_seconds;
    state->drying_timeout_seconds = settings->drying_timeout_seconds;
    state->discharge_seconds = settings->discharge_seconds;
    state->logging_interval_seconds = settings->logging_interval_seconds;
    return ESP_OK;
}
