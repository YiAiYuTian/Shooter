#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "event_type.h"

#include <vector>
#include <unordered_map>
#include <utility>
#include <functional>
#include <type_traits>
#include <memory>

namespace shooter
{

struct EventHandle
{
    EventTypeID id;
    Uint64 unique_id;

    constexpr bool operator==(const EventHandle& other) const noexcept
    {
        return id == other.id && unique_id == other.unique_id;
    }
    constexpr bool operator!=(const EventHandle& other) const noexcept
    {
        return !(*this == other);
    }
};
constexpr EventHandle INVALID_EVENT_HANDLE = { INVALID_EVENT_TYPE_ID, 0 };

using EventCallback = std::function<void(const IEvent &)>;

class EventManager
{
public:
    static EventManager &instance()
    {
        static EventManager em;
        return em;
    }

    static void update()
    {
        instance().update_impl();
    }

    template <typename T, typename Fn>
    requires std::is_base_of_v<IEvent, T>    
    static EventHandle subscribe(Fn &&func)
    {
        EventCallback cb =
            [func_move = std::forward<Fn>(func)](const IEvent & e)
            {
                const T &typed_event = static_cast<const T &>(e);
                func_move(typed_event);
            };
        return instance().subscribe_impl(T::type_id, std::move(cb));
    }

    static void unsubscribe(EventHandle hd)
    {
        instance().unsubscribe_impl(hd);
    }

    template <typename T>
    requires std::is_base_of_v<IEvent, T>
    static void enqueue(const T & e)
    {
        instance().enqueue_impl(e);
    }

    template <typename T>
    requires std::is_base_of_v<IEvent, T>
    static void enqueue(T &&e)
    {
        instance().enqueue_impl(std::move(e));
    }
private:
    EventHandle subscribe_impl(EventTypeID id, EventCallback cb);
    void unsubscribe_impl(EventHandle hd);

    template <typename T>
    void enqueue_impl(const T& e)
    {
        m_event_queue.emplace_back(std::make_unique<T>(e));
    }

    template <typename T>
    void enqueue_impl(T &&e)
    {
        m_event_queue.emplace_back(std::make_unique<T>(std::move(e)));
    }

    void update_impl();
private:
    std::unordered_map<EventTypeID, std::vector<std::pair<Uint64, EventCallback>>> m_events;
    Uint64 m_next_handle = 1;
    std::vector<std::unique_ptr<IEvent>> m_event_queue;
};

}    

#endif // !EVENT_MANAGER_H
