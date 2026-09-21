#include "hmi_calibration.h"

#include <stdio.h>
#include <string.h>

void hmi_calibration_init(hmi_calibration_t *calibration)
{
    if (calibration == NULL) {
        return;
    }

    memset(calibration, 0, sizeof(*calibration));
    calibration->saved = false;
}

bool hmi_calibration_add_point(hmi_calibration_t *calibration, float voltage_v, float moisture_percent, char *error, size_t error_size)
{
    if (calibration == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Calibration pointer is NULL");
        }
        return false;
    }

    if (voltage_v <= 0.0f || voltage_v > 5.0f) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Voltage must be between 0 V and 5 V");
        }
        return false;
    }

    if (moisture_percent < 0.0f || moisture_percent > 100.0f) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Moisture value must be between 0 %% and 100 %%");
        }
        return false;
    }

    if (calibration->point_count >= HMI_CALIBRATION_MAX_POINTS) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Calibration point limit reached");
        }
        return false;
    }

    calibration->points[calibration->point_count].voltage_v = voltage_v;
    calibration->points[calibration->point_count].moisture_percent = moisture_percent;
    calibration->point_count++;
    calibration->saved = false;

    if (error != NULL && error_size > 0) {
        error[0] = '\0';
    }
    return true;
}

float hmi_calibration_interpolate(const hmi_calibration_t *calibration, float voltage_v)
{
    if (calibration == NULL || calibration->point_count == 0) {
        return 0.0f;
    }

    if (calibration->point_count == 1) {
        return calibration->points[0].moisture_percent;
    }

    size_t lower_index = 0;
    size_t upper_index = 0;
    for (size_t index = 0; index < calibration->point_count; index++) {
        if (calibration->points[index].voltage_v <= voltage_v) {
            lower_index = index;
        }
        if (calibration->points[index].voltage_v >= voltage_v) {
            upper_index = index;
            break;
        }
    }

    if (upper_index == lower_index) {
        return calibration->points[lower_index].moisture_percent;
    }

    const float x0 = calibration->points[lower_index].voltage_v;
    const float x1 = calibration->points[upper_index].voltage_v;
    const float y0 = calibration->points[lower_index].moisture_percent;
    const float y1 = calibration->points[upper_index].moisture_percent;

    if (x1 == x0) {
        return y0;
    }

    return y0 + (voltage_v - x0) * (y1 - y0) / (x1 - x0);
}

esp_err_t hmi_calibration_apply(hmi_calibration_t *calibration, const hmi_calibration_t *source)
{
    if (calibration == NULL || source == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (source->point_count > HMI_CALIBRATION_MAX_POINTS) {
        return ESP_ERR_INVALID_SIZE;
    }

    memcpy(calibration, source, sizeof(*calibration));
    calibration->saved = true;
    return ESP_OK;
}
