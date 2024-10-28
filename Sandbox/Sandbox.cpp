#include "Application.h"

class Sandbox : public Application {
public:
    Sandbox() {
        //Log::log("Sandbox application initialized.", Log::Level::INFO);
    }

protected:
    void Run() override {
        //Log::log("Sandbox application is running...", Log::Level::INFO);
        // Sandbox application logic here
    }
};

// Factory function to create the application instance
Application* CreateApplication() {
    return new Sandbox();
}
