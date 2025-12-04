#pragma once

#include "common.h"
#include "weather.h"

static void am_inbox_received_callback(DictionaryIterator *iterator, void *context) {
    weather_inbox_received_callback(iterator, context);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "inbox received");
}

static void am_inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
    s_weather_request_in_progress = false;
}

static void am_outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
    s_weather_request_in_progress = false;
}

static void am_outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void am_init() {
    // Register JS callbacks.
    app_message_register_inbox_received(am_inbox_received_callback);
    app_message_register_inbox_dropped(am_inbox_dropped_callback);
    app_message_register_outbox_failed(am_outbox_failed_callback);
    app_message_register_outbox_sent(am_outbox_sent_callback);

    // Open AppMessage.
    const int inbox_size = 512;
    const int outbox_size = 256;
    app_message_open(inbox_size, outbox_size);
}
