#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

#include "ui_base.h"

namespace shooter
{

class ProgressBar : public UIBase
{
public:
    ProgressBar() = default;

    void set_progress(float p);
    void set_color(glm::vec4 color);
    void set_delay_color(glm::vec4 color);
    void set_follow_speed(float speed);

    void on_update(float dt) override;
    void on_event() override;
    void on_render() override;
private:
    float m_progress = 0.0f;
    float m_display = 0.0f;
    float m_follow_speed = 3.0f;
    glm::vec4 m_color = { 0.196f, 0.784f, 0.196f, 1.0f };
    glm::vec4 m_delay_color = { 1.0f, 0.863f, 0.196f, 1.0f };
};

}    

#endif // !PROGRESS_BAR_H
