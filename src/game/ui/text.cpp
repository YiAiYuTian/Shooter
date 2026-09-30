#include "text.h"
#include "../../render/render_manager.h"

namespace shooter
{

void Text::set_text(const char *text)
{
    m_text = text ? text : "";
}

void Text::set_font(TTF_Font *font)
{
    m_font = font;
}

void Text::set_font_size(float size)
{
    m_font_size = size;
}

void Text::set_color(glm::vec4 color)
{
    m_color = color;
}

void Text::on_update(float dt)
{
    (void)dt;
    UIBase::on_update(dt);
}

void Text::on_event()
{
    UIBase::on_event();
}

void Text::on_render()
{
    if (!m_font || m_text.empty()) return;

    RenderManager::draw_text(
        m_text.c_str(), m_font,
        { (int)(m_world_rect.x), (int)(m_world_rect.y) },
        m_font_size, m_color
    );

    UIBase::on_render();
}

}
