#include "hmi_faults.h"

#include <string.h>

void hmi_faults_init(hmi_state_t *state)
{
    if (state == NULL) {
        return;
    }

    memset(state->active_faults, 0, sizeof(state->active_faults));
    state->active_fault_count = 0;
}

bool hmi_faults_add(hmi_state_t *state, hmi_fault_severity_t severity, const char *description, time_t timestamp)
{
    if (state == NULL || description == NULL) {
        return false;
    }

    if (state->active_fault_count >= HMI_MAX_ACTIVE_FAULTS) {
        return false;
    }

    hmi_fault_t *fault = &state->active_faults[state->active_fault_count];
    fault->id = (uint32_t)(state->active_fault_count + 1);
    fault->severity = severity;
    fault->active = true;
    fault->timestamp = timestamp;
    snprintf(fault->description, sizeof(fault->description), "%s", description);
    state->active_fault_count++;
    return true;
}

void hmi_faults_clear(hmi_state_t *state)
{
    if (state == NULL) {
        return;
    }

    memset(state->active_faults, 0, sizeof(state->active_faults));
    state->active_fault_count = 0;
}

int hmi_faults_count(const hmi_state_t *state)
{
    if (state == NULL) {
        return 0;
    }

    return (int)state->active_fault_count;
}

bool hmi_faults_has_critical(const hmi_state_t *state)
{
    if (state == NULL) {
        return false;
    }

    for (size_t index = 0; index < state->active_fault_count; index++) {
        if (state->active_faults[index].severity == HMI_FAULT_CRITICAL) {
            return true;
        }
    }

    return false;
}
