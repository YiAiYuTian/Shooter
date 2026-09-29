#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include <glm/glm.hpp>

struct SDL_Renderer;
struct SDL_Window;
struct SDL_Texture;
struct TTF_Font;

namespace shooter
{

class RenderManager
{
public:
    static RenderManager &instance()
    {
        static RenderManager rm;
        return rm;
    }

    static bool init(SDL_Window *window)
    {
        return instance().init_impl(window);
    }

    static void quit()
    {
        instance().quit_impl();
    }

    static SDL_Renderer *renderer()
    {
        return instance().renderer_impl();
    }

    static void begin_frame(glm::vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f })
    {
        instance().begin_frame_impl(color);
    }

    static void end_frame()
    {
        instance().end_frame_impl();
    }

    static void draw_rect(glm::vec4 rect, glm::vec4 color)
    {
        instance().draw_rect_impl(rect, color);
    }

    static void draw_fill_rect(glm::vec4 rect, glm::vec4 color)
    {
        instance().draw_rect_fill_impl(rect, color);
    }

    static void draw_texture(SDL_Texture *texture, glm::vec4 *src_rect, glm::vec4 dst_rect)
    {
        instance().draw_texture_impl(texture, src_rect, dst_rect);
    }

    static void draw_text(const char *text, TTF_Font *font, glm::ivec2 pos, float size, glm::vec4 color)
    {
        instance().draw_text_impl(text, font, pos, size, color);
    }

private:
    RenderManager() = default;
    ~RenderManager() = default;

    bool init_impl(SDL_Window *window);
    void quit_impl();
    SDL_Renderer *renderer_impl();
    
    void begin_frame_impl(glm::vec4 color);
    void end_frame_impl();

    void draw_rect_impl(glm::vec4 rect, glm::vec4 color);
    void draw_rect_fill_impl(glm::vec4 rect, glm::vec4 color);
    void draw_texture_impl(SDL_Texture *texture, glm::vec4 *src_rect, glm::vec4 dst_rect);
    void draw_text_impl(const char *text, TTF_Font *font, glm::ivec2 pos, float size, glm::vec4 color);
private:
    struct Impl;
    Impl *m_impl = nullptr;
};

}    

#endif // !RENDER_MANAGER_H
