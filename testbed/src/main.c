#include <core/logger.h>
#include <core/asserts.h>
#include <platform/platform.h>

int main(void){
    FATAL("test message : %f", 2.0f);
    WARN("test message : %f", 3.0f);

    //ASSERT_MSG(1 == 0, "FALSE");
    platform_state state;
    if(platform_startup(&state, "Game Engine", 100, 100, 1280, 720)){
        while(TRUE){
            platform_pump_messages(&state);
        }
    }
    platform_shutdown(&state);
    return 0;
}
