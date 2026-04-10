#include "input.h"

#include <stdlib.h>

#include "game.h"
#include "core/logger.h"

b8 input_initialize(game *game) {
    INFO("Input system initialized.");

    if (event_register(EVENT_CODE_KEY_PRESSED, game->state, input_on_key_event)) return FALSE;
    if (event_register(EVENT_CODE_KEY_RELEASED, game->state, input_on_key_event)) return FALSE;

    return TRUE;
}

b8 input_on_key_event(u16 code, void* sender, void* listener_inst, event_context context) {

    switch (code) {
        case EVENT_CODE_KEY_PRESSED: {
            u16 key_code = context.data.u16[0];
            switch (key_code) {
                case 'A' :
                case 'a' : {
                    INFO("'A' key pressed");
                }
            }
        }
        case EVENT_CODE_KEY_RELEASED: {
            u16 key_code = context.data.u16[0];
            switch (key_code) {}
        }
    }

    return FALSE;
}

b8 input_on_mouse_event(u16 code, void* sender, void* listener_inst, event_context context) {
    (void)code;
    (void)sender;
    (void)listener_inst;
    (void)context;
    return FALSE;
}

b8 input_shutdown(game* game) {
    INFO("Input system shutting down.");
    event_unregister(EVENT_CODE_KEY_PRESSED, game->state, input_on_key_event);
    event_unregister(EVENT_CODE_KEY_RELEASED, game->state, input_on_key_event);
    return TRUE;
}