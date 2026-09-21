#include <assert.h>

#include "hmi/hmi_rtc.h"

int main(void)
{
    assert(hmi_rtc_is_valid_datetime((int64_t)1769000000));
    assert(!hmi_rtc_is_valid_datetime((int64_t)-1));
    assert(!hmi_rtc_is_valid_datetime((int64_t)0));
    return 0;
}
