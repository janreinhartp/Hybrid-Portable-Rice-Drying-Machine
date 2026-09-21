#include "board_port.h"

#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "touch_input.h"

static const char *TAG = "board_port";
static SemaphoreHandle_t lvgl_mutex;
static TaskHandle_t lvgl_task_handle;
static esp_lcd_panel_handle_t lcd_panel;
static lv_disp_draw_buf_t lvgl_draw_buffer;
static lv_color_t *frame_buffer_a;
static lv_color_t *frame_buffer_b;

static void lvgl_tick_callback(void *argument)
{
    (void)argument;
    lv_tick_inc(2);
}

static void lvgl_flush_callback(lv_disp_drv_t *display_driver, const lv_area_t *area, lv_color_t *color_map)
{
    const int32_t width = area->x2 - area->x1 + 1;
    const int32_t height = area->y2 - area->y1 + 1;

    esp_lcd_panel_draw_bitmap(lcd_panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1,
                              color_map);
    (void)width;
    (void)height;
    lv_disp_flush_ready(display_driver);
}

static esp_err_t init_lcd_panel(void)
{
    const esp_lcd_rgb_panel_config_t panel_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .timings = {
            .pclk_hz = BOARD_LCD_PIXEL_CLOCK_HZ,
            .h_res = BOARD_LCD_H_RES,
            .v_res = BOARD_LCD_V_RES,
            .hsync_pulse_width = 162,
            .hsync_back_porch = 152,
            .hsync_front_porch = 48,
            .vsync_pulse_width = 45,
            .vsync_back_porch = 13,
            .vsync_front_porch = 3,
            .flags.pclk_active_neg = 1,
        },
        .data_width = 16,
        .bits_per_pixel = 16,
        .num_fbs = 2,
        .bounce_buffer_size_px = BOARD_LCD_H_RES * 10,
        .hsync_gpio_num = GPIO_NUM_46,
        .vsync_gpio_num = GPIO_NUM_3,
        .de_gpio_num = GPIO_NUM_5,
        .pclk_gpio_num = GPIO_NUM_7,
        .disp_gpio_num = GPIO_NUM_NC,
        .data_gpio_nums = {
            GPIO_NUM_14, GPIO_NUM_38, GPIO_NUM_18, GPIO_NUM_17, GPIO_NUM_10,
            GPIO_NUM_39, GPIO_NUM_0, GPIO_NUM_45, GPIO_NUM_48, GPIO_NUM_47,
            GPIO_NUM_21, GPIO_NUM_1, GPIO_NUM_2, GPIO_NUM_42, GPIO_NUM_41,
            GPIO_NUM_40,
        },
        .flags.fb_in_psram = 1,
    };

    ESP_RETURN_ON_ERROR(esp_lcd_new_rgb_panel(&panel_config, &lcd_panel), TAG,
                        "failed to create RGB panel");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(lcd_panel), TAG, "failed to initialize RGB panel");

    ESP_RETURN_ON_ERROR(esp_lcd_rgb_panel_get_frame_buffer(lcd_panel, 2,
                                                            (void **)&frame_buffer_a,
                                                            (void **)&frame_buffer_b),
                        TAG, "failed to get RGB framebuffers");
    return ESP_OK;
}

static esp_err_t init_lvgl(void)
{
    lv_init();
    lvgl_mutex = xSemaphoreCreateRecursiveMutex();
    if (lvgl_mutex == NULL) {
        return ESP_ERR_NO_MEM;
    }

    lv_disp_draw_buf_init(&lvgl_draw_buffer, frame_buffer_a, frame_buffer_b,
                          BOARD_LCD_H_RES * BOARD_LCD_V_RES);

    lv_disp_drv_t display_driver;
    lv_disp_drv_init(&display_driver);
    display_driver.hor_res = BOARD_LCD_H_RES;
    display_driver.ver_res = BOARD_LCD_V_RES;
    display_driver.flush_cb = lvgl_flush_callback;
    display_driver.draw_buf = &lvgl_draw_buffer;
    display_driver.full_refresh = 1;
    if (lv_disp_drv_register(&display_driver) == NULL) {
        return ESP_FAIL;
    }

    const esp_timer_create_args_t tick_timer_args = {
        .callback = lvgl_tick_callback,
        .name = "lvgl_tick",
    };
    esp_timer_handle_t tick_timer;
    ESP_RETURN_ON_ERROR(esp_timer_create(&tick_timer_args, &tick_timer), TAG,
                        "failed to create LVGL tick timer");
    ESP_RETURN_ON_ERROR(esp_timer_start_periodic(tick_timer, 2000), TAG,
                        "failed to start LVGL tick timer");
    return ESP_OK;
}

static void lvgl_task(void *argument)
{
    (void)argument;
    while (true) {
        if (xSemaphoreTakeRecursive(lvgl_mutex, portMAX_DELAY) == pdTRUE) {
            uint32_t next_delay_ms = lv_timer_handler();
            xSemaphoreGiveRecursive(lvgl_mutex);
            vTaskDelay(pdMS_TO_TICKS(next_delay_ms < 5 ? 5 : next_delay_ms));
        }
    }
}

esp_err_t board_port_init(void)
{
    ESP_RETURN_ON_ERROR(init_lcd_panel(), TAG, "LCD initialization failed");
    ESP_RETURN_ON_ERROR(init_lvgl(), TAG, "LVGL initialization failed");
    ESP_RETURN_ON_ERROR(touch_input_init(), TAG, "GT911 initialization failed");

    lv_indev_drv_t input_driver;
    lv_indev_drv_init(&input_driver);
    input_driver.type = LV_INDEV_TYPE_POINTER;
    input_driver.read_cb = touch_input_read;
    if (lv_indev_drv_register(&input_driver) == NULL) {
        return ESP_FAIL;
    }

    if (xTaskCreate(lvgl_task, "lvgl", 8192, NULL, 2, &lvgl_task_handle) != pdPASS) {
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "RGB panel and LVGL task initialized at %dx%d", BOARD_LCD_H_RES, BOARD_LCD_V_RES);
    return ESP_OK;
}

bool board_lvgl_lock(int timeout_ms)
{
    if (lvgl_mutex == NULL) {
        return false;
    }

    const TickType_t timeout = timeout_ms < 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvgl_mutex, timeout) == pdTRUE;
}

void board_lvgl_unlock(void)
{
    if (lvgl_mutex != NULL) {
        xSemaphoreGiveRecursive(lvgl_mutex);
    }
}
