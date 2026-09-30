#include "app.h"
#include "logger.h"
#include "resource_manager.h"
#include "../event/event_type.h"
#include "../event/input_manager.h"
#include "../render/render_manager.h"

#include <SDL3/SDL.h>
#include <chrono>
#include <thread>

namespace shooter
{

class Time
{
    using Clock = std::chrono::high_resolution_clock;
public:
    Time() { m_last = Clock::now(); }

    void set_target_fps(int fps)
    {
        if (fps <= 0)
        {
            m_target_frame = std::chrono::duration<double>(0.0);
            return;
        }
        m_target_frame = std::chrono::duration<double>(1.0 / static_cast<double>(fps));
    }

    void end_frame()
    {
        auto now = Clock::now();
        auto elapsed = now - m_last;

        m_delta_time = std::chrono::duration<double>(elapsed).count();

        if (m_target_frame.count() > 0.0 && elapsed < m_target_frame)
        {
            auto sleep_time = m_target_frame - elapsed;
            std::this_thread::sleep_for(sleep_time);
        }

        m_last = now;
    }

    [[nodiscard]] float get_delta() const
    {
        return static_cast<float>(m_delta_time);
    }

    void disable_limit()
    {
        m_target_frame = std::chrono::duration<double>(0.0);
    }

private:
    Clock::time_point m_last;
    std::chrono::duration<double> m_target_frame{0.0};
    double m_delta_time{1.0 / 60.0};
};

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

    Time time;
    time.set_target_fps(60);
    while (m_impl->is_running)
    {
        float dt = time.get_delta();
        on_event();
        on_update(dt);
        on_render();

        time.end_frame();
    }

    quit();
    return 0;
}

void App::quit_game()
{
    m_impl->is_running = false;
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

void App::on_update(float dt)
{
    EventManager::update();

    m_game.on_update(dt);
}

void App::on_render()
{
    RenderManager::begin_frame();
    // render
    m_game.on_render();

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

    // game load
    if (!m_game.init(this)) return false;

    m_impl->is_running = true;
    return true;
}

void App::quit()
{
    m_game.quit();

    RenderManager::quit();
    SDL_DestroyWindow(m_impl->window);
    ResourceManager::quit();
    SDL_Quit();

    delete m_impl;
}    

}    

