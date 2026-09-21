#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "esp_err.h"
#include "hmi_types.h"

void hmi_faults_init(hmi_state_t *state);
bool hmi_faults_add(hmi_state_t *state, hmi_fault_severity_t severity, const char *description, time_t timestamp);
void hmi_faults_clear(hmi_state_t *state);
int hmi_faults_count(const hmi_state_t *state);
bool hmi_faults_has_critical(const hmi_state_t *state);
