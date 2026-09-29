#include "app.h"
#include "logger.h"
#include "resource_manager.h"
#include "../event/event_type.h"
#include "../event/input_manager.h"
#include "../render/render_manager.h"

#include <SDL3/SDL.h>

namespace shooter
{

struct App::Impl
{
    bool is_running = false;
    const char *title = "Shooter";
    int screen_width = 1280;
    int screen_height = 720;

    SDL_Window *window = nullptr;
};

int App::run(int argc, char **argv)
{
    (void)argc, (void)argv;

    if (!init()) return -1;

    while (m_impl->is_running)
    {
        on_event();
        on_update();
        on_render();
    }

    quit();
    return 0;
}

void App::on_event()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.quit.type == SDL_EVENT_QUIT) m_impl->is_running = false;
        switch (e.type)
        {
        case SDL_EVENT_QUIT:
        {
            QuitEvent event;
            EventManager::enqueue(event);
            break;
        }
        case SDL_EVENT_KEY_DOWN:
            InputManager::on_sdl_key_event(&e.key);
            break;
        case SDL_EVENT_KEY_UP:
            InputManager::on_sdl_key_event(&e.key);
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            InputManager::on_sdl_mouse_btn_event(&e.button);
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            InputManager::on_sdl_mouse_btn_event(&e.button);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            InputManager::on_sdl_mouse_motion_event(&e.motion);
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            InputManager::on_sdl_mouse_wheel_event(&e.wheel);
            break;
        default:
            break;
        }
    }
}

void App::on_update()
{
    EventManager::update();

    
}

void App::on_render()
{
    RenderManager::begin_frame();
    // render

    RenderManager::end_frame();
}

bool App::init()
{
    m_impl = new App::Impl();    

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LOG_FATAL("Failed to initialize SDL: {}", SDL_GetError());
        return false;
    }
    if (!ResourceManager::init()) return false;

    if (m_impl->window = SDL_CreateWindow(m_impl->title, m_impl->screen_width, m_impl->screen_height, 0); !m_impl->window)
    {
        LOG_FATAL("Failed to initialize window and renderer: {}", SDL_GetError());
        return false;
    }
    if (!RenderManager::init(m_impl->window)) return false;

    // load resource
    ResourceManager::set_renderer(RenderManager::renderer());
    if (!ResourceManager::load_resource("./assets.pak")) return false;

    // Quit App Event
    EventManager::subscribe<QuitEvent>(
        [&](const QuitEvent &e)
        {
            m_impl->is_running = false;
        }
    );

    EventManager::subscribe<KeyEvent>(
        [&](const KeyEvent &e)
        {
            if (e.key == SDLK_ESCAPE)
                m_impl->is_running = false;
        }
    );

    m_impl->is_running = true;
    return true;
}

void App::quit()
{
    RenderManager::quit();
    SDL_DestroyWindow(m_impl->window);
    ResourceManager::quit();
    SDL_Quit();

    delete m_impl;
}    

}    

