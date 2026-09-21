#include "hmi_diagnostics.h"

#include <string.h>

void hmi_diagnostics_init(hmi_diagnostics_t *diagnostics)
{
    if (diagnostics == NULL) {
        return;
    }

    memset(diagnostics, 0, sizeof(*diagnostics));
    diagnostics->lvgl_refresh_interval_ms = 250;
}

esp_err_t hmi_diagnostics_from_state(hmi_diagnostics_t *diagnostics, const hmi_state_t *state)
{
    if (diagnostics == NULL || state == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    diagnostics->sd_available = state->sd_available;
    diagnostics->rtc_valid = state->rtc_valid;
    diagnostics->active_fault_count = (uint32_t)state->active_fault_count;
    diagnostics->psram_available = true;
    diagnostics->free_heap_bytes = 0;
    diagnostics->free_psram_bytes = 0;
    diagnostics->lvgl_refresh_interval_ms = 250;
    return ESP_OK;
}
