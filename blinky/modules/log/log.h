#ifndef MY_MODULE_LOG
#define MY_MODULE_LOG

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_log_backend_usb.h"


void logs_init();

void logs_empty_action();


#endif