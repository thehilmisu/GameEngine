#include "Application.h"

void Application::Run(Application* appInstance) {
    if (appInstance) {
        //Log::log("Application starting...", Log::Level::INFO);
        appInstance->Run();  // Call the overridden Run function of the derived class
        //Log::log("Application exiting...", Log::Level::INFO);
        delete appInstance;  // Clean up after execution
    } else {
        //Log::log("No application instance provided.", Log::Level::ERROR);
    }
}
