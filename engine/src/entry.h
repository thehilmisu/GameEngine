#pragma once

#include <core/application.h>
#include <core/logger.h>
#include <game_types.h>

//external function to create the game
extern b8 create_game(game* out_game);


int main(void){

    game game_instance;
    if(!create_game(&game_instance)){
        FATAL("Could not create the GAME!");
        return -1;
    }

    if(!game_instance.render || !game_instance.update || 
        !game_instance.initialize || !game_instance.onresize){
        FATAL("The game's function pointers must be assigned!");
        return -2;
    }


   
    if(!application_create(&game_instance)){
        INFO("Application failed to create!");
        return 1;
    }

    if(!application_run()){
        INFO("Application did not shutdown normally");
        return 2;
    }

    application_run();

    return 0;
}
