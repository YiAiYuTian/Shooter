#ifndef TEXT_H
#define TEXT_H

#include "ui_base.h"

#include <string>

struct TTF_Font;

namespace shooter
{

class Text : public UIBase
{
public:
    Text() = default;

    void set_text(const char *text);
    void set_font(TTF_Font *font);
    void set_font_size(float size);
    void set_color(glm::vec4 color);

    void on_update(float dt) override;
    void on_event() override;
    void on_render() override;
private:
    std::string m_text;
    TTF_Font *m_font = nullptr;
    float m_font_size = 22.0f;
    glm::vec4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

}    

#endif // !TEXT_H
