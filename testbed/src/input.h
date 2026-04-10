#ifndef GAMEENGINE_INPUT_H
#define GAMEENGINE_INPUT_H

#include "definitions.h"
#include "core/event.h"
#include "game_types.h"

b8 input_initialize(game *game);
b8 input_shutdown(game* game);
b8 input_on_key_event(u16 code, void* sender, void* listener_inst, event_context context);
b8 input_on_mouse_event(u16 code, void* sender, void* listener_inst, event_context context);

#endif //GAMEENGINE_INPUT_H
