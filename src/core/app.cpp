#include "app.h"
#include "logger.h"
#include "../event/event_type.h"
#include "../event/input_manager.h"

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
    SDL_Renderer *renderer = nullptr;
};    

int App::run_impl(int argc, char **argv)
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
    SDL_Renderer *rd = m_impl->renderer;

    SDL_SetRenderDrawColorFloat(rd, 0.0f, 0.0f, 0.0f, 1.0f);
    SDL_RenderClear(rd);

    // render
    

    SDL_RenderPresent(rd);
}

bool App::init()
{
    m_impl = new App::Impl();    

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LOG_FATAL("Failed to initialize SDL!");
        return false;
    }

    if (!SDL_CreateWindowAndRenderer(m_impl->title, m_impl->screen_width, m_impl->screen_height, 0, &m_impl->window, &m_impl->renderer))
    {
        LOG_FATAL("Failed to initialize window and renderer!");
        return false;
    }

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
    SDL_DestroyWindow(m_impl->window);
    SDL_DestroyRenderer(m_impl->renderer);
    SDL_Quit();

    delete m_impl;
}    

}    

