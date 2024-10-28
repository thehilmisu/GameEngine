#include "Application.h"

#include "Events/ApplicationEvent.h"
#include "Log.h"

Application::Application()
{
}


Application::~Application()
{
}

void Application::Run()
{
    WindowResizeEvent e(1280, 720);
    if (e.IsInCategory(EventCategoryApplication))
    {
        CLIENT_TRACE(e.ToString());
    }
    if (e.IsInCategory(EventCategoryInput))
    {
        CLIENT_TRACE(e.ToString());
    }

    while (true);
}
