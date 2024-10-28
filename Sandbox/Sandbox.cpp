#include "Application.h"

class Sandbox : public Application
{
public:
	Sandbox()
	{

	}

	~Sandbox()
	{

	}

};

// Factory function to create the application instance
Application* CreateApplication() {
    return new Sandbox();
}
