/* Minimal OS stub to provide lv_os_get_idle_percent when no RTOS is used */
#include <stdint.h>

uint32_t lv_os_get_idle_percent(void)
{
    /* No RTOS idle tracking on this port. Return 0% idle. */
    return 0;
}
