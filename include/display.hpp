#pragma once

#include <SDL.h>

class Display {
    public:
        static const int SCALE_FACTOR = 4;

        Display() {}

        bool initSdl();
        void shutdownSdl();

        void clear();
        void render(const uint8_t *pixels);
        void refresh();
        
    private:
        bool sdlInited = false;
        SDL_Window* window;
        SDL_Renderer* renderer;
};
