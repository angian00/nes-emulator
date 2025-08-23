#include "display.hpp"

#include "ppu.hpp"

#include <print>
#include <cstdio>



bool Display::initSystemPalette(const char* palFile)
{
    FILE *f;
    if (!(f = fopen(palFile, "rb")))
    {
        std::println("!! {} is not a readable file", palFile);
        return false;
    }

    auto nRead = fread(m_systemPalette, sizeof(uint8_t), 64*3, f);    
    fclose(f);

    if (nRead != 64*3) {
        std::println("!! not a valid palette file");
        return false;
    }

    // dumpSystemPaletteEntry(0x0F);
    // dumpSystemPaletteEntry(0x15);
    // dumpSystemPaletteEntry(0x2C);
    // dumpSystemPaletteEntry(0x12);

    return true;
}

bool Display::initSdl()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::println("SDL could not be initialized!");
        std::println("SDL_Error: {}", SDL_GetError());
        return false;
    }
    sdlInited = true;

    auto windowWidth  = Ppu::SCREEN_WIDTH  * SCALE_FACTOR;
    auto windowHeight = Ppu::SCREEN_HEIGHT * SCALE_FACTOR;

    window = SDL_CreateWindow("NES Emulator by AnGian",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                windowWidth, windowHeight,
                                SDL_WINDOW_SHOWN);
    if (!window)
    {
        std::println("Window could not be created!");
        std::println("SDL_Error: {}", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::println("Renderer could not be created!");
        std::println("SDL_Error: {}", SDL_GetError());
        return false;
    }

    clear();

    return true;
}

void Display::shutdownSdl()
{
    if (renderer)
        SDL_DestroyRenderer(renderer);

    if (window)
        SDL_DestroyWindow(window);

    if (sdlInited)
        SDL_Quit();
}

void Display::clear()
{
    uint8_t* clearColor = (uint8_t*)m_systemPalette;
    SDL_SetRenderDrawColor(renderer, clearColor[0], clearColor[1], clearColor[2], 0xff);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}


void Display::render(const uint8_t *pixels)
{
    //clear();

    for (auto x=0; x < Ppu::SCREEN_WIDTH; x++)
    {
        for (auto y=0; y < Ppu::SCREEN_HEIGHT; y++)
        {
            uint8_t colorIndex = pixels[x*Ppu::SCREEN_HEIGHT + y];
            uint8_t* color = m_systemPalette + 3*colorIndex;
            SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], 0xff);

            SDL_Rect rect(x*SCALE_FACTOR, y*SCALE_FACTOR, SCALE_FACTOR, SCALE_FACTOR);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    
    SDL_RenderPresent(renderer);
}


void Display::refresh()
{
    SDL_RenderPresent(renderer);
    SDL_RaiseWindow(window);
}


void Display::dumpSystemPaletteEntry(uint8_t paletteIndex)
{
    std::println("system palette entry 0x{:02X}: (0x{:02X}, 0x{:02X}, 0x{:02X})", paletteIndex, 
        m_systemPalette[3*paletteIndex], m_systemPalette[3*paletteIndex+1], m_systemPalette[3*paletteIndex+2]);
}
