#include "Application.h"

// Declare the CreateApplication function here, defined in the client
extern Application* CreateApplication();

int main() 
{
    Log::Init();
    CORE_WARN("Initialized Log");
    int a =3; 
    CLIENT_INFO("Hello! var={0}", a);

    auto app = CreateApplication();
    app->Run();
    delete app;
    
    return 0;
}
