#include "platform.h"
#include "../core/types.h"
#include "../../render/render_manager.h"

namespace shooter
{

void Platform::draw() const
{
    float x = get_pos().x;
    float y = get_pos().y;
    float w = this->get_w();
    float h = this->get_h();
    if (w < 30.0f && h > 100.0f)
    {
        RenderManager::draw_fill_rect({ x, y, w, h }, WALL_BODY);
        RenderManager::draw_fill_rect({ x, y, 3.0f, h }, WALL_LIT);
        RenderManager::draw_fill_rect({ x + w - 3.0f, y, 3.0f, h }, WALL_DARK);
    }
    else if (h >= 20.0f && w > 200.0f)
    {
        RenderManager::draw_fill_rect({ x, y, w, h }, PLAT_BODY);
        RenderManager::draw_fill_rect({ x, y, w, 3.0f }, PLAT_TOP);
    }
    else
    {
        RenderManager::draw_fill_rect({ x, y, w, h }, PLAT_BODY);
        RenderManager::draw_fill_rect({ x, y, 4.0f, h }, PLAT_EDGE);
        RenderManager::draw_fill_rect({ x + w - 4.0f, y, 4.0f, h }, PLAT_EDGE);
    }
}

}