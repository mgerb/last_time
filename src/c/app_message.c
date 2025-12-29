#include "app_message.h"
#include "assert.h"
#include "log.h"
#include "pebble.h"
#include "settings.h"
#include "time.h"
#include "weather.h"

static AM_MESSAGE_TYPE am_get_message_type(DictionaryIterator *iterator) {
    if (dict_find(iterator, MESSAGE_KEY_config_temperature)) {
        return AM_CONFIG;
    }

    if (dict_find(iterator, MESSAGE_KEY_temperature_f)) {
        return AM_WEATHER;
    }

    return AM_UNKNOWN;
}

static void am_inbox_received_callback(DictionaryIterator *iterator, void *context) {
    AM_MESSAGE_TYPE message_type = am_get_message_type(iterator);
    LOG_DEBUG("Inbox received, message type: %d", (int)message_type);

    switch (message_type) {
    case AM_CONFIG:
        settings_update_from_message(iterator);
        // Refresh the temperature and time because the
        // format depends on the settings.
        weather_refresh_temperature();
        time_update();
        break;
    case AM_WEATHER:
        weather_inbox_received_callback(iterator, context);
        break;
    case AM_UNKNOWN:
        ASSERT(!"This should never happen. You probably modified known message payloads.");
    }
}

static void am_inbox_dropped_callback(AppMessageResult reason, void *context) {
    LOG_ERROR("Message dropped!");
    weather_request_reset_state();
}

static void am_outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    LOG_ERROR("Outbox send failed!");
    weather_request_reset_state();
}

static void am_outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    LOG_INFO("Outbox send success!");
}

void am_init(void) {
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
