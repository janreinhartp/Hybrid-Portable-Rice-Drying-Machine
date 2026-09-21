#include "hmi_state.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static hmi_state_t current_state;
static SemaphoreHandle_t state_mutex;

esp_err_t hmi_state_init(void)
{
    state_mutex = xSemaphoreCreateMutex();
    if (state_mutex == NULL) {
        return ESP_ERR_NO_MEM;
    }

    memset(&current_state, 0, sizeof(current_state));
    current_state.machine_state = HMI_MACHINE_IDLE;
    current_state.sd_available = false;
    current_state.temperature_setpoint_c = 50.0f;
    current_state.maximum_temperature_c = 70.0f;
    current_state.temperature_hysteresis_c = 2.0f;
    current_state.target_moisture_percent = 14.0f;
    current_state.moisture_filtered = 18.0f;
    current_state.moisture_confirmation_seconds = 30;
    current_state.drying_timeout_seconds = 3600;
    current_state.discharge_seconds = 30;
    current_state.logging_interval_seconds = 10;
    return ESP_OK;
}

esp_err_t hmi_state_read(hmi_state_t *state)
{
    if (state == NULL || state_mutex == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(state_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    memcpy(state, &current_state, sizeof(*state));
    xSemaphoreGive(state_mutex);
    return ESP_OK;
}

esp_err_t hmi_state_publish(const hmi_state_t *state)
{
    if (state == NULL || state_mutex == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(state_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    memcpy(&current_state, state, sizeof(current_state));
    xSemaphoreGive(state_mutex);
    return ESP_OK;
}
