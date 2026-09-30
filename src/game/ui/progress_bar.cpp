#include "progress_bar.h"
#include "../core/types.h"
#include "../../render/render_manager.h"

namespace shooter
{

void ProgressBar::set_progress(float p)
{
    m_progress = p < 0.0f ? 0.0f : (p > 1.0f ? 1.0f : p);
}

void ProgressBar::set_color(glm::vec4 color)
{
    m_color = color;
}

void ProgressBar::set_delay_color(glm::vec4 color)
{
    m_delay_color = color;
}

void ProgressBar::set_follow_speed(float speed)
{
    m_follow_speed = speed > 0.0f ? speed : 0.0f;
}

void ProgressBar::on_update(float dt)
{
    if (m_display != m_progress)
    {
        float step = m_follow_speed * dt;
        if (m_display > m_progress)
            m_display = m_display - step < m_progress ? m_progress : m_display - step;
        else
            m_display = m_display + step > m_progress ? m_progress : m_display + step;
    }
}

void ProgressBar::on_event()
{
}

void ProgressBar::on_render()
{
    RenderManager::draw_fill_rect(m_world_rect, PANEL);
    if (m_display > 0.0f)
        RenderManager::draw_fill_rect({ m_world_rect.x, m_world_rect.y, m_world_rect.z * m_display, m_world_rect.w }, m_delay_color);
    if (m_progress > 0.0f)
        RenderManager::draw_fill_rect({ m_world_rect.x, m_world_rect.y, m_world_rect.z * m_progress, m_world_rect.w }, m_color);
    RenderManager::draw_rect(m_world_rect, WHITE);
}

}    
