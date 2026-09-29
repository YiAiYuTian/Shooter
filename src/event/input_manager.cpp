#include "input_manager.h"

#include <SDL3/SDL.h>

namespace shooter
{

bool InputManager::is_key_held_impl(SDL_Scancode k)
{
    const bool *keyboard = SDL_GetKeyboardState(nullptr);
    return keyboard[k];
}

bool InputManager::is_mouse_held_impl(SDL_MouseButtonFlags m)
{
    SDL_MouseButtonFlags flags = SDL_GetMouseState(nullptr, nullptr);
    return (flags & m) != 0;
}

void InputManager::on_sdl_mouse_btn_event_impl(SDL_MouseButtonEvent *e)
{
    MouseButtonEvent event{};
    event.button = e->button;
    event.clicks = e->clicks;
    event.down = e->down;
    event.x = e->x;
    event.y = e->y;
    EventManager::enqueue(event);
}

void InputManager::on_sdl_mouse_motion_event_impl(SDL_MouseMotionEvent *e)
{
    MouseMotionEvent event{};
    event.x = e->x;
    event.y = e->y;
    event.xrel = e->xrel;
    event.yrel = e->yrel;
    EventManager::enqueue(event);
}

void InputManager::on_sdl_mouse_wheel_event_impl(SDL_MouseWheelEvent *e)
{
    MouseWheelEvent event{};
    event.x = e->x;
    event.y = e->y;
    event.mouse_x = e->mouse_x;
    event.mouse_y = e->mouse_y;
    EventManager::enqueue(event);
}

void InputManager::on_sdl_key_event_impl(SDL_KeyboardEvent *e)
{
    KeyEvent event{};
    event.scancode = e->scancode;
    event.key = e->key;
    event.mod = e->mod;
    event.down = e->down;
    event.repeat = e->repeat;
    EventManager::enqueue(event);
}

}    
