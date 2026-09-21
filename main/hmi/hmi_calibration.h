#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "esp_err.h"

#define HMI_CALIBRATION_MAX_POINTS 16

typedef struct {
    float voltage_v;
    float moisture_percent;
} hmi_calibration_point_t;

typedef struct {
    hmi_calibration_point_t points[HMI_CALIBRATION_MAX_POINTS];
    size_t point_count;
    bool saved;
} hmi_calibration_t;

void hmi_calibration_init(hmi_calibration_t *calibration);
bool hmi_calibration_add_point(hmi_calibration_t *calibration, float voltage_v, float moisture_percent, char *error, size_t error_size);
float hmi_calibration_interpolate(const hmi_calibration_t *calibration, float voltage_v);
esp_err_t hmi_calibration_apply(hmi_calibration_t *calibration, const hmi_calibration_t *source);
