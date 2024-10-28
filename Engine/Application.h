// Application.h
#ifndef APPLICATION_H
#define APPLICATION_H

#include "Log.h"

class Application {
public:
    virtual ~Application() = default;

    // Static start method to encapsulate main function logic
    static void Run(Application* appInstance);

protected:
    // Constructor is protected to prevent direct instantiation
    Application() = default;

    // Pure virtual function to be implemented by client subclasses
    virtual void Run() = 0;
};

#endif // APPLICATION_H


