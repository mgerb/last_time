#include "app_message.h"
#include "settings.h"
#include "time.h"
#include "weather.h"

// TODO: create a discriminator
static bool is_config_message(DictionaryIterator *iterator) {
    return dict_find(iterator, MESSAGE_KEY_config_temperature) || dict_find(iterator, MESSAGE_KEY_config_date_format) ||
           dict_find(iterator, MESSAGE_KEY_config_vibrate_disconnect) ||
           dict_find(iterator, MESSAGE_KEY_config_vibrate_top_hour);
}

static void am_inbox_received_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Inbox received");

    if (is_config_message(iterator)) {
        settings_update_from_message(iterator);
        weather_refresh_temperature();
        time_update();
        return;
    }

    weather_inbox_received_callback(iterator, context);
}

static void am_inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
    weather_request_reset_state();
}

static void am_outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
    weather_request_reset_state();
}

static void am_outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
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
