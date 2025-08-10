#pragma once

#include <SDL.h>

class Display {
    public:
        static const int SCALE_FACTOR = 4;

        Display() {}

        bool initSdl();
        void shutdownSdl();

        bool initSystemPalette(const char* palFile);

        void clear();
        void render(const uint8_t *pixels);
        void refresh();

        void dumpSystemPaletteEntry(uint8_t paletteIndex);

        
    private:
        uint8_t m_systemPalette[64*3];

        bool sdlInited = false;
        SDL_Window* window;
        SDL_Renderer* renderer;
};
