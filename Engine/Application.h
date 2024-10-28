// Application.h
#ifndef APPLICATION_H
#define APPLICATION_H

#include "Log.h"

class Application 
{
public:
    Application();
    virtual ~Application();
    
    void Run();
};

Application* CreateApplication();

#endif // APPLICATION_H


