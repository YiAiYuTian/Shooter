#ifndef ANIMATION_H
#define ANIMATION_H

#include <glm/glm.hpp>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>
#include <memory>


struct SDL_Texture;

namespace shooter
{

class Atlas
{
public:
    Atlas(std::initializer_list<std::string_view> list);
    Atlas(const std::vector<std::string> &names);
    Atlas(Atlas &&atlas) noexcept;
    ~Atlas() = default;

    SDL_Texture *get_texture(int idx);
    size_t size() const;
private:
    std::vector<SDL_Texture *> m_textures;
};

class Animation
{
public:
    Animation(Atlas &&atlas);
    ~Animation() = default;

    void on_update(float dt);
    void on_render();

    SDL_Texture *current() const { return m_current; }

    void set_interval(float time);
    void set_pos(glm::vec2 pos);
    void set_size(glm::vec2 size);
private:
    void to_next_frame();
private:
    Atlas m_atlas;
    SDL_Texture *m_current = nullptr;
    int m_idx = 0;
    float m_timer = 0.0f;
    float m_interval = 0.0f;

    glm::vec4 m_pos_size{};
};

}    

#endif // !ANIMATION_H
