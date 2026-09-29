#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>
#include <string_view>
#include <unordered_map>

struct SDL_Renderer;
struct SDL_Texture;
struct TTF_Font;

namespace shooter
{

class ResourceManager
{
public:
    static ResourceManager &instance()
    {
        static ResourceManager rm;
        return rm;
    }

    static bool init()
    {
        return instance().init_impl();
    }
    
    static void quit()
    {
        instance().quit_impl();
    }

    static void set_renderer(SDL_Renderer *renderer)
    {
        instance().m_renderer = renderer;
    }

    // load xxx.pak
    static bool load_resource(std::string_view path)
    {
        return instance().load_resource_impl(path);
    }

    static SDL_Texture *get_texture(std::string_view name)
    {
        return instance().get_texture_impl(name);
    }

    static TTF_Font *get_font(std::string_view name)
    {
        return instance().get_font_impl(name);
    }        
private:
    ResourceManager() = default;
    ~ResourceManager() = default;

    bool init_impl();
    void quit_impl();

    bool load_resource_impl(std::string_view path);
    SDL_Texture *get_texture_impl(std::string_view name);
    TTF_Font *get_font_impl(std::string_view name);
private:
    SDL_Renderer *m_renderer = nullptr;
    std::unordered_map<std::string, SDL_Texture *> m_textures;
    std::unordered_map<std::string, TTF_Font *> m_fonts;
};

}    

#endif // !RESOURCE_MANAGER_H
