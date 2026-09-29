#ifndef APP_H
#define APP_H

namespace shooter
{

class App
{
public:
    App() = default;
    ~App() = default;
    
    int run(int argc, char **argv);
private:
    bool init();
    void quit();

    void on_event();
    void on_update();
    void on_render();
private:
    struct Impl;
    Impl *m_impl = nullptr;
};

}

#endif // !APP_H
