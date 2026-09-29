#ifndef EVENT_TYPE_H
#define EVENT_TYPE_H

#include "../core/base_type.h"

#include <SDL3/SDL_events.h>

namespace shooter
{

using EventTypeID = Uint32;
inline EventTypeID g_event_type_counter = 1;
constexpr EventTypeID INVALID_EVENT_TYPE_ID = 0;

class IEvent
{
public:
    virtual ~IEvent() = default;

    [[nodiscard]] virtual EventTypeID get_event_type_id() const noexcept = 0;
    [[nodiscard]] virtual const char *get_type_name() const noexcept = 0;
protected:
    constexpr IEvent() noexcept = default;
public:
    mutable bool consumed = false;
};

template <typename T>
class EventBase : public IEvent
{
public:
    [[nodiscard]] EventTypeID get_event_type_id() const noexcept override
    {
        return type_id;
    }

    [[nodiscard]] const char *get_type_name() const noexcept override
    {
        return T::type_name;
    }
public:
    static inline const EventTypeID type_id = g_event_type_counter++;
};

#define REGISTER_EVENT_BEGIN(name)                            \
    class name : public shooter::EventBase<name>              \
    {                                                         \
    public:                                                   \
        static constexpr const char *const type_name = #name;
#define REGISTER_EVENT_END(name) \
    };

// events
REGISTER_EVENT_BEGIN(QuitEvent)
REGISTER_EVENT_END(QuitEvent)

REGISTER_EVENT_BEGIN(MouseButtonEvent)
    Uint8 button;
    Uint8 clicks;
    bool down;
    float x, y;
REGISTER_EVENT_END(MouseButtonEvent)

REGISTER_EVENT_BEGIN(MouseMotionEvent)
    float x;
    float y;
    float xrel;
    float yrel;
REGISTER_EVENT_END(MouseMotionEvent)

REGISTER_EVENT_BEGIN(MouseWheelEvent)
    float x;
    float y;
    float mouse_x;
    float mouse_y;
REGISTER_EVENT_END(MouseWheelEvent)

REGISTER_EVENT_BEGIN(KeyEvent)
    SDL_Scancode scancode;
    Uint32 key;
    Uint16 mod;
    bool down;
    bool repeat;
REGISTER_EVENT_END(KeyEvent)

}

#endif // !EVENT_TYPE_H
