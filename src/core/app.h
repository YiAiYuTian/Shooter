#ifndef APP_H
#define APP_H

namespace shooter
{

class App
{
public:
    static App &instance()
    {
        static App app;
        return app;
    }

    static int run(int argc, char **argv)
    {
        return instance().run_impl(argc, argv);
    }
private:
    App() = default;
    ~App() = default;

    int run_impl(int argc, char **argv);

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
