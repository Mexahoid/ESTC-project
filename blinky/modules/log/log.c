#include "log.h"

void logs_init()
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

void logs_process()
{
    LOG_BACKEND_USB_PROCESS();
    NRF_LOG_PROCESS();
}
