#pragma once

#include <stdbool.h>

#include "esp_err.h"

#define BOARD_LCD_H_RES 1024
#define BOARD_LCD_V_RES 600
#define BOARD_LCD_PIXEL_CLOCK_HZ 30850000

esp_err_t board_port_init(void);
bool board_lvgl_lock(int timeout_ms);
void board_lvgl_unlock(void);
