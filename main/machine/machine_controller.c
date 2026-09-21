#include "machine_controller.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "hmi_state.h"

static const char *TAG = "machine_controller";
static TaskHandle_t controller_task_handle;

static hmi_actuator_status_t *actuator_status_for(hmi_state_t *state, hmi_actuator_t actuator)
{
    switch (actuator) {
    case HMI_ACTUATOR_ELEVATOR:
        return &state->elevator;
    case HMI_ACTUATOR_FAN:
        return &state->fan;
    case HMI_ACTUATOR_HEATER:
        return &state->heater;
    case HMI_ACTUATOR_DISCHARGE:
        return &state->discharge;
    default:
        return NULL;
    }
}

static void set_actuators_off(hmi_state_t *state, const char *reason)
{
    hmi_actuator_status_t *actuators[] = {
        &state->elevator,
        &state->fan,
        &state->heater,
        &state->discharge,
    };

    for (size_t index = 0; index < sizeof(actuators) / sizeof(actuators[0]); index++) {
        actuators[index]->requested = HMI_ACTUATOR_OFF;
        actuators[index]->actual = HMI_ACTUATOR_OFF;
        strlcpy(actuators[index]->reason, reason, sizeof(actuators[index]->reason));
    }
}

static void controller_task(void *argument)
{
    hmi_command_t command;
    (void)argument;

    while (true) {
        if (hmi_command_receive(&command, portMAX_DELAY) == ESP_OK) {
            esp_err_t result = machine_controller_process(&command);
            if (result != ESP_OK && result != ESP_ERR_NOT_SUPPORTED) {
                ESP_LOGW(TAG, "Rejected HMI command type %d: %s", (int)command.type, esp_err_to_name(result));
            }
        }
    }
}

esp_err_t machine_controller_init(void)
{
    hmi_state_t initial_state;

    if (hmi_state_read(&initial_state) != ESP_OK) {
        return ESP_ERR_INVALID_STATE;
    }

    set_actuators_off(&initial_state, "NO REQUEST");
    if (hmi_state_publish(&initial_state) != ESP_OK) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Machine controller boundary initialized");
    return ESP_OK;
}

esp_err_t machine_controller_start(void)
{
    if (controller_task_handle != NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    return xTaskCreate(controller_task, "machine_controller", 4096, NULL, 8, &controller_task_handle) == pdPASS
        ? ESP_OK
        : ESP_ERR_NO_MEM;
}

esp_err_t machine_controller_process(const hmi_command_t *command)
{
    hmi_state_t state;

    if (command == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (hmi_state_read(&state) != ESP_OK) {
        return ESP_ERR_INVALID_STATE;
    }

    if (command->type == HMI_COMMAND_EMERGENCY_STOP) {
        state.machine_state = HMI_MACHINE_EMERGENCY_STOP;
        set_actuators_off(&state, "EMERGENCY STOP");
        return hmi_state_publish(&state);
    }

    if (state.machine_state == HMI_MACHINE_EMERGENCY_STOP) {
        return ESP_ERR_INVALID_STATE;
    }

    if (command->type == HMI_COMMAND_SET_ACTUATOR) {
        hmi_actuator_status_t *status = actuator_status_for(&state, command->payload.actuator.actuator);
        if (status == NULL) {
            return ESP_ERR_INVALID_ARG;
        }

        status->requested = command->payload.actuator.on ? HMI_ACTUATOR_ON : HMI_ACTUATOR_OFF;
        if (command->payload.actuator.on) {
            status->actual = HMI_ACTUATOR_INTERLOCKED;
            strlcpy(status->reason, "ACTUATOR DRIVER NOT CONNECTED", sizeof(status->reason));
            if (command->payload.actuator.actuator == HMI_ACTUATOR_HEATER) {
                strlcpy(status->reason, "SAFETY SERVICES NOT CONNECTED", sizeof(status->reason));
            }
            hmi_state_publish(&state);
            return ESP_ERR_NOT_SUPPORTED;
        }

        status->actual = HMI_ACTUATOR_OFF;
        strlcpy(status->reason, "OFF BY CONTROLLER", sizeof(status->reason));
        return hmi_state_publish(&state);
    }

    return ESP_ERR_NOT_SUPPORTED;
}
