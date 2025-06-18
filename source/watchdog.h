#pragma once
#include <ctime>
#define __QOR_WATCHDOG_SRC__

#include "rendering_env.h"

struct __Watchdog
{
    qo_bool_t   alarmed;
    qo_int32_t  timeout_milliseconds;
    clock_t     last_tick;
};
typedef struct __Watchdog _Watchdog;

void
watchdog_feed(
    _Watchdog * self
) {
    self->alarmed = qo_true;
    self->last_tick = clock();
}

void
watchdog_disarm(
    _Watchdog * self
) {
    self->alarmed = qo_false;
}

qo_bool_t
watchdog_is_timeout(
    _Watchdog * self
) {
    clock_t current_tick = clock();
    clock_t elapsed_ticks = current_tick - self->last_tick;
    qo_int32_t elapsed_milliseconds = elapsed_ticks * 1000 / CLOCKS_PER_SEC;
    return elapsed_milliseconds > self->timeout_milliseconds;
}


