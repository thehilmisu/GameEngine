#pragma once

#include "definitions.h"

struct game;

// config struct basically the window features
typedef struct application_config {
    i16 start_pos_x;
    i16 start_pos_y;
    i16 start_width;
    i16 start_height;
    char* name;
} application_config;

API b8 application_create(struct game* game_instance);

API b8 application_run();
