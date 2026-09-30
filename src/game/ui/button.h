#ifndef BUTTON_H
#define BUTTON_H

#include "ui_base.h"
#include "../../event/event_manager.h"

#include <glm/glm.hpp>
#include <functional>

struct SDL_Texture;

namespace shooter
{

constexpr glm::vec4 NROMAL_COLOR =  { 1.0f, 1.0f, 1.0f, 0.5f };
constexpr glm::vec4 HOVERED_COLOR = { 0.627f, 0.627f, 0.627f, 0.5f };
constexpr glm::vec4 PRESSED_COLOR = { 1.0f, 0.863f, 0.196f, 0.5f };

class Button : public UIBase
{
    using Callback = std::function<void()>;
public:
    Button();
    ~Button() override;

    void on_update(float dt) override;
    void on_event() override;
    void on_render() override;

    void set_callback(Callback cb);
    void set_hovered_callback(Callback cb);
    void set_texture(SDL_Texture *normal, SDL_Texture *hover, SDL_Texture *click);

    bool is_hovered() const { return m_is_hovered; }
private:
    SDL_Texture *m_textures[3]{};
    bool m_is_hovered = false;
    bool m_is_pressed = false;
    Callback m_callback;
    Callback m_hovered_callback;
    EventHandle m_handle = INVALID_EVENT_HANDLE;
};

}    

#endif // !BUTTON_H
