#include <assert.h>
#include <time.h>

#include "hmi/hmi_faults.h"

int main(void)
{
    hmi_state_t state = {0};
    hmi_faults_init(&state);

    assert(hmi_faults_add(&state, HMI_FAULT_CRITICAL, "Heater fault", time(NULL)));
    assert(hmi_faults_count(&state) == 1);
    assert(hmi_faults_has_critical(&state));

    hmi_faults_clear(&state);
    assert(hmi_faults_count(&state) == 0);
    assert(!hmi_faults_has_critical(&state));
    return 0;
}
