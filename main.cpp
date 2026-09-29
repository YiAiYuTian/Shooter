#include "src/core/app.h"

#include <SDL3/SDL_main.h>

int main(int argc, char **argv)
{
    shooter::App app;
    return app.run(argc, argv);
}
