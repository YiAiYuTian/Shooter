#include "animation.h"
#include "../../core/resource_manager.h"
#include "../../render/render_manager.h"
#include "../../core/logger.h"

namespace shooter
{

Atlas::Atlas(std::initializer_list<std::string_view> list)
{
    for (auto name : list)
    {
        auto *tex = ResourceManager::get_texture(name);
        if (!tex) continue;
        
        m_textures.push_back(tex);
    }
}

Atlas::Atlas(const std::vector<std::string> &names)
{
    for (const auto &name : names)
    {
        auto *tex = ResourceManager::get_texture(name);
        if (!tex) continue;
        m_textures.push_back(tex);
    }
}

Atlas::Atlas(Atlas &&atlas) noexcept
{
    m_textures = std::move(atlas.m_textures);
}

SDL_Texture *Atlas::get_texture(int idx)
{
    if (idx >= m_textures.size()) return nullptr;
    
    return m_textures[idx];
}

size_t Atlas::size() const
{
    return m_textures.size();
}

Animation::Animation(Atlas &&atlas)
    : m_atlas(std::move(atlas)), m_idx(0)
{
    if (m_atlas.size() != 0)
        m_current = m_atlas.get_texture(m_idx++);
}

void Animation::on_update(float dt)
{
    m_timer += dt;
    if (m_timer >= m_interval)
    {
        to_next_frame();
        m_timer -= m_interval;
    }
}

void Animation::on_render()
{
    RenderManager::draw_texture(m_current, nullptr, m_pos_size);
}

void Animation::set_interval(float time)
{
    m_interval = time;
}

void Animation::set_pos(glm::vec2 pos)
{
    m_pos_size.x = pos.x;
    m_pos_size.y = pos.y;
}

void Animation::set_size(glm::vec2 size)
{
    m_pos_size.z = size.x;
    m_pos_size.w = size.y;
}

void Animation::to_next_frame()
{
    if (m_atlas.size() == 0) return;
    m_current = m_atlas.get_texture(m_idx++ % m_atlas.size());
}

}    
