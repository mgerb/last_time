#pragma once

typedef enum {
    AM_UNKNOWN = 0,
    AM_CONFIG = 1,
    AM_WEATHER = 2,
} AM_MESSAGE_TYPE;

void am_init(void);
