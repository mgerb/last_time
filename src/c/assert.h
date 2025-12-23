#pragma once

#include <pebble.h>

#ifdef DEBUG
#define ASSERT(cond)                                                                                                   \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            APP_LOG(APP_LOG_LEVEL_ERROR, "ASSERT FAILED: %s (%s:%d)", #cond, __FILE__, __LINE__);                      \
            *(volatile int *)0 = 0;                                                                                    \
        }                                                                                                              \
    } while (0)
#else
#define ASSERT(cond) ((void)0)
#endif
