#include <core/logger.h>
#include <core/asserts.h>

int main(void){
    FATAL("test message : %f", 2.0f);
    WARN("test message : %f", 3.0f);

    ASSERT_MSG(1 == 0, "FALSE");

    return 0;
}
