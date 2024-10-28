#include "Application.h"

// Declare the CreateApplication function here, defined in the client
extern Application* CreateApplication();

int main() 
{
    Log::Init();
    Log::GetCoreLogger()->warn("Initialized Log");
    Log::GetClientLogger()->info("Hello!");

    Application* app = CreateApplication();
    Application::Run(app);
    delete app;
    return 0;
}
