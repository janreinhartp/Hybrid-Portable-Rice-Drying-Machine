#include "hmi_command.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static QueueHandle_t command_queue;

esp_err_t hmi_command_init(size_t queue_length)
{
    if (queue_length == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    command_queue = xQueueCreate(queue_length, sizeof(hmi_command_t));
    return command_queue == NULL ? ESP_ERR_NO_MEM : ESP_OK;
}

esp_err_t hmi_command_send(const hmi_command_t *command, uint32_t timeout_ms)
{
    if (command == NULL || command_queue == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    return xQueueSend(command_queue, command, pdMS_TO_TICKS(timeout_ms)) == pdTRUE
        ? ESP_OK
        : ESP_ERR_TIMEOUT;
}

esp_err_t hmi_command_receive(hmi_command_t *command, uint32_t timeout_ms)
{
    if (command == NULL || command_queue == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    return xQueueReceive(command_queue, command, pdMS_TO_TICKS(timeout_ms)) == pdTRUE
        ? ESP_OK
        : ESP_ERR_TIMEOUT;
}
