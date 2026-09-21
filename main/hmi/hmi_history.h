#pragma once

#include <stddef.h>
#include <stdbool.h>

#include "esp_err.h"

#define HMI_HISTORY_MAX_ENTRIES 32

typedef struct {
    char session_id[24];
    float moisture_percent;
    float upper_temperature_c;
    float lower_temperature_c;
    int64_t timestamp_seconds;
} hmi_history_entry_t;

typedef struct {
    hmi_history_entry_t entries[HMI_HISTORY_MAX_ENTRIES];
    size_t count;
} hmi_history_t;

void hmi_history_init(hmi_history_t *history);
bool hmi_history_append(hmi_history_t *history, const hmi_history_entry_t *entry);
size_t hmi_history_count(const hmi_history_t *history);
esp_err_t hmi_history_trim(hmi_history_t *history, size_t max_entries);
