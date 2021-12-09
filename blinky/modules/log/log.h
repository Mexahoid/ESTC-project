#ifndef MY_MODULE_LOG
#define MY_MODULE_LOG

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_log_backend_usb.h"

/**
 * @brief Initialize method.
 *
 */
void logs_init();

/**
 * @brief Just an empty action (it doesn't work without it).
 *
 */
void logs_process();


#endif