#include <assert.h>
#include <string.h>

#include "hmi/hmi_calibration.h"

int main(void)
{
    hmi_calibration_t calibration;
    char error[128] = {0};

    hmi_calibration_init(&calibration);
    assert(hmi_calibration_add_point(&calibration, 1.20f, 25.0f, error, sizeof(error)));
    assert(hmi_calibration_add_point(&calibration, 1.55f, 20.0f, error, sizeof(error)));
    assert(hmi_calibration_add_point(&calibration, 1.84f, 18.5f, error, sizeof(error)));
    assert(hmi_calibration_add_point(&calibration, 2.20f, 15.0f, error, sizeof(error)));

    const float interpolated = hmi_calibration_interpolate(&calibration, 1.84f);
    assert(interpolated > 18.0f && interpolated < 19.0f);

    hmi_calibration_t copy = {0};
    assert(hmi_calibration_apply(&copy, &calibration) == ESP_OK);
    assert(copy.saved);
    assert(copy.point_count == 4);

    return 0;
}
