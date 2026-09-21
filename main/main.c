#include "board_port.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hmi_command.h"
#include "hmi_state.h"
#include "machine_controller.h"
#include "screen_manager.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_ERROR_CHECK(board_port_init());
    ESP_ERROR_CHECK(hmi_state_init());
    ESP_ERROR_CHECK(hmi_command_init(16));
    ESP_ERROR_CHECK(machine_controller_init());
    ESP_ERROR_CHECK(machine_controller_start());

    /* The board port owns LVGL initialization and task synchronization. */
    ESP_ERROR_CHECK(board_lvgl_lock(-1));
    ESP_ERROR_CHECK(screen_manager_init());
    board_lvgl_unlock();
    ESP_LOGI(TAG, "Rice dryer HMI scaffold started");
}
