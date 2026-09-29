#include "event_manager.h"

#include <algorithm>

namespace shooter
{

EventHandle EventManager::subscribe_impl(EventTypeID id, EventCallback cb)
{
    Uint64 unique_id = m_next_handle++;
    m_events[id].emplace_back(unique_id, std::move(cb));
    return { id, unique_id };
}

void EventManager::unsubscribe_impl(EventHandle hd)
{
    auto it = m_events.find(hd.id);
    if (it == m_events.end()) return;

    auto &event_list = it->second;

    event_list.erase(
        std::remove_if(event_list.begin(), event_list.end(),
           [&](const std::pair<Uint64, EventCallback> &pair) -> bool
           {
               return pair.first == hd.unique_id;
           }),
        event_list.end()
    );

    if (event_list.empty()) m_events.erase(it);
}

void EventManager::update_impl()
{
    auto queue = std::move(m_event_queue);
    m_event_queue.clear();

    for (auto &e : queue)
    {
        auto it = m_events.find(e->get_event_type_id());
        if (it == m_events.end()) continue;

        auto callbacks = it->second;
        for (auto &[u_id, cb] : callbacks)
        {
            cb(*e);
            if (e->consumed) break;
        }
    }
}

}    
