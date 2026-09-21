#pragma once

#include <stdbool.h>

#include "esp_err.h"
#include "hmi_types.h"

typedef struct {
    bool sd_available;
    bool rtc_valid;
    bool psram_available;
    uint32_t free_heap_bytes;
    uint32_t free_psram_bytes;
    uint32_t active_fault_count;
    int32_t lvgl_refresh_interval_ms;
} hmi_diagnostics_t;

void hmi_diagnostics_init(hmi_diagnostics_t *diagnostics);
esp_err_t hmi_diagnostics_from_state(hmi_diagnostics_t *diagnostics, const hmi_state_t *state);
