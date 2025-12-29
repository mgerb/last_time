#pragma once

#include <pebble.h>

#define LOG_ERROR(...) APP_LOG(APP_LOG_LEVEL_ERROR, __VA_ARGS__)

// Debug-only logging. Removed in release builds.
#ifdef DEBUG
#define LOG_DEBUG(...) APP_LOG(APP_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(...) APP_LOG(APP_LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_WARN(...) APP_LOG(APP_LOG_LEVEL_WARNING, __VA_ARGS__)
#else
#define LOG_DEBUG(...) ((void)0)
#define LOG_INFO(...) ((void)0)
#define LOG_WARN(...) ((void)0)
#endif
