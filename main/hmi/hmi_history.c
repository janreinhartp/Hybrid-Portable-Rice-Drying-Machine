#include "hmi_history.h"

#include <string.h>

void hmi_history_init(hmi_history_t *history)
{
    if (history == NULL) {
        return;
    }

    memset(history, 0, sizeof(*history));
}

bool hmi_history_append(hmi_history_t *history, const hmi_history_entry_t *entry)
{
    if (history == NULL || entry == NULL) {
        return false;
    }

    if (history->count >= HMI_HISTORY_MAX_ENTRIES) {
        return false;
    }

    history->entries[history->count] = *entry;
    history->count++;
    return true;
}

size_t hmi_history_count(const hmi_history_t *history)
{
    if (history == NULL) {
        return 0;
    }

    return history->count;
}

esp_err_t hmi_history_trim(hmi_history_t *history, size_t max_entries)
{
    if (history == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (max_entries > HMI_HISTORY_MAX_ENTRIES) {
        return ESP_ERR_INVALID_SIZE;
    }

    if (history->count <= max_entries) {
        return ESP_OK;
    }

    const size_t remove_count = history->count - max_entries;
    memmove(&history->entries[0], &history->entries[remove_count], sizeof(history->entries[0]) * (max_entries));
    history->count = max_entries;
    return ESP_OK;
}
