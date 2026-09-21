#pragma once

#include "esp_err.h"
#include "lvgl.h"

esp_err_t touch_input_init(void);
void touch_input_read(lv_indev_drv_t *driver, lv_indev_data_t *data);
