#include <assert.h>
#include <string.h>

#include "hmi/hmi_settings.h"

int main(void)
{
    hmi_settings_t settings;
    hmi_state_t state = {0};
    char error[128] = {0};

    hmi_settings_init(&settings);
    assert(hmi_settings_validate(&settings, error, sizeof(error)));
    assert(error[0] == '\0');

    settings.temperature_setpoint_c = 300.0f;
    assert(!hmi_settings_validate(&settings, error, sizeof(error)));
    assert(strstr(error, "Temperature setpoint") != NULL);

    hmi_settings_init(&settings);
    settings.target_moisture_percent = 1.0f;
    assert(!hmi_settings_validate(&settings, error, sizeof(error)));
    assert(strstr(error, "Target moisture") != NULL);

    hmi_settings_init(&settings);
    settings.temperature_setpoint_c = 62.0f;
    settings.maximum_temperature_c = 82.0f;
    settings.temperature_hysteresis_c = 2.5f;
    settings.target_moisture_percent = 12.5f;
    settings.moisture_confirmation_seconds = 45;
    settings.drying_timeout_seconds = 5400;
    settings.discharge_seconds = 45;
    settings.logging_interval_seconds = 15;

    assert(hmi_settings_apply(&state, &settings) == ESP_OK);
    assert(state.temperature_setpoint_c == 62.0f);
    assert(state.maximum_temperature_c == 82.0f);
    assert(state.temperature_hysteresis_c == 2.5f);
    assert(state.target_moisture_percent == 12.5f);
    assert(state.moisture_confirmation_seconds == 45U);
    assert(state.drying_timeout_seconds == 5400U);
    assert(state.discharge_seconds == 45U);
    assert(state.logging_interval_seconds == 15U);

    return 0;
}
