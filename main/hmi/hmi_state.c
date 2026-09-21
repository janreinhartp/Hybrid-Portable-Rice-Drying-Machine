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
