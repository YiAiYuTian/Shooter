#ifndef UI_BASE_H
#define UI_BASE_H

#include <glm/glm.hpp>
#include <vector>

namespace shooter
{

class UIBase
{
public:
    UIBase() = default;
    virtual ~UIBase()
    {
        for (auto *ui : m_children)
            delete ui;
        m_children.clear();
    }

    template<typename T, typename... Args>
    T* add_child(Args&&... args)
    {
        T* child = new T(std::forward<Args>(args)...);
        m_children.push_back(child);
        return child;
    }

    virtual void on_update(float dt)
    {
        for (auto *ui : m_children)
            ui->on_update(dt);
    }

    virtual void on_event()
    {
        for (auto *ui : m_children)
            ui->on_event();
    }

    virtual void on_render()
    {
        for (auto *ui : m_children)
            ui->on_render();
    }

    virtual void set_layout(glm::vec4 parent_pos)
    {
        m_world_rect.x = parent_pos.x + m_rect.x;
        m_world_rect.y = parent_pos.y + m_rect.y;
        m_world_rect.z = m_rect.z;
        m_world_rect.w = m_rect.w;

        for (auto *ui : m_children)
        {
            ui->set_layout(m_world_rect);
        }
    }

    void set_pos(glm::vec2 pos) { m_rect.x = pos.x; m_rect.y = pos.y; }
    void set_size(glm::vec2 size) { m_rect.z = size.x; m_rect.w = size.y; }

    glm::vec4 get_rect() const { return m_rect; }
    glm::vec4 get_world_rect() const { return m_world_rect; }
protected:
    glm::vec4 m_rect{};
    glm::vec4 m_world_rect{};
    std::vector<UIBase*> m_children;
};

}    

#endif // !UI_BASE_H
