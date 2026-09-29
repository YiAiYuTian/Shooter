#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "event_manager.h"

namespace shooter
{

class InputManager
{
public:
    static InputManager &instance()
    {
        static InputManager im;
        return im;
    }

    static bool is_key_held(SDL_Scancode k)
    {
        return instance().is_key_held_impl(k);
    }

    static bool is_mouse_held(SDL_MouseButtonFlags m)
    {
        return instance().is_mouse_held_impl(m);
    }

    static void on_sdl_mouse_btn_event(SDL_MouseButtonEvent *e)
    {
        instance().on_sdl_mouse_btn_event_impl(e);
    }

    static void on_sdl_mouse_motion_event(SDL_MouseMotionEvent *e)
    {
        instance().on_sdl_mouse_motion_event_impl(e);
    }        

    static void on_sdl_mouse_wheel_event(SDL_MouseWheelEvent *e)
    {
        instance().on_sdl_mouse_wheel_event_impl(e);
    }        

    static void on_sdl_key_event(SDL_KeyboardEvent *e)
    {
        instance().on_sdl_key_event_impl(e);
    }
private:
    bool is_key_held_impl(SDL_Scancode k);
    bool is_mouse_held_impl(SDL_MouseButtonFlags m);

    void on_sdl_mouse_btn_event_impl(SDL_MouseButtonEvent *e);
    void on_sdl_mouse_motion_event_impl(SDL_MouseMotionEvent *e);
    void on_sdl_mouse_wheel_event_impl(SDL_MouseWheelEvent *e);
    void on_sdl_key_event_impl(SDL_KeyboardEvent *e);
};

}    

#endif // !INPUT_MANAGER_H
