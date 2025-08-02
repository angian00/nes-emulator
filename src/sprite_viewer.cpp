
#include "cartridge.hpp"

#include <print>
#include <SDL.h>


static const int PTABLE_WIDTH  = 128;
static const int PTABLE_HEIGHT = 128;
static const int SCALE_FACTOR = 6;

const uint8_t COLOR_BKG[] = { 0x00, 0x00, 0x00, 0xff };
const uint8_t COLOR_1[]   = { 0xcc, 0x00, 0x00, 0xff };
const uint8_t COLOR_2[]   = { 0x00, 0x00, 0xcc, 0xff };
const uint8_t COLOR_3[]   = { 0xcc, 0xcc, 0xcc, 0xff };
const uint8_t COLOR_KO[]  = { 0xff, 0x00, 0xff, 0xff };

enum PatternTableIndex {
    Left=0,
    Right=1
};

bool initSdl();
void shutdownSdl();

void clearScreen();
void refresh();

void loadPatternTable(Cartridge* cart, uint8_t (&screenData)[PTABLE_WIDTH][PTABLE_HEIGHT], PatternTableIndex iTable);
void renderPatternTable(uint8_t (&screenData)[PTABLE_WIDTH][PTABLE_HEIGHT], PatternTableIndex iTable);


bool sdlInited = false;
SDL_Window* window;
SDL_Renderer* renderer;


int main(int argc, char* argv[])
{
    std::println("-- NES CHR viewer by AnGian");

    if (argc < 2) {
        std::println("!! Missing ROM path");
        exit(1);
    }

    auto cart = new Cartridge(argv[1]);
    uint8_t screenData[PTABLE_WIDTH][PTABLE_HEIGHT] = {};

    if (!initSdl())
    {
        shutdownSdl();
        exit(1);
    }

    loadPatternTable(cart, screenData, PatternTableIndex::Left);
    renderPatternTable(screenData, PatternTableIndex::Left);
    loadPatternTable(cart, screenData, PatternTableIndex::Right);
    renderPatternTable(screenData, PatternTableIndex::Right);

    bool running = true;
    SDL_Event e;
    while (running)
    {
        if (SDL_PollEvent(&e) && (e.type == SDL_QUIT))
            running = false;
    }
}


bool initSdl()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::println("!! SDL could not be initialized!");
        std::println("SDL_Error: {}", SDL_GetError());
        return false;
    }
    sdlInited = true;

    auto SCREEN_WIDTH = 2 * PTABLE_WIDTH * SCALE_FACTOR;
    auto SCREEN_HEIGHT = PTABLE_HEIGHT * SCALE_FACTOR;

    window = SDL_CreateWindow("NES CHR Viewer by AnGian",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window)
    {
        std::println("!! Window could not be created!");
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

    clearScreen();

    return true;
}


void shutdownSdl()
{
    if (renderer)
        SDL_DestroyRenderer(renderer);

    if (window)
        SDL_DestroyWindow(window);

    if (sdlInited)
        SDL_Quit();
}


void clearScreen()
{
    SDL_SetRenderDrawColor(renderer, COLOR_BKG[0], COLOR_BKG[1], COLOR_BKG[2], COLOR_BKG[3]);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void refresh()
{
    SDL_RenderPresent(renderer);
    //SDL_RaiseWindow(window);
}



void loadPatternTable(Cartridge* cart, uint8_t (&screenData)[PTABLE_WIDTH][PTABLE_HEIGHT], 
        PatternTableIndex iTable)
{
    static int iChrBlock = 0;

    static int wTiles = 0x010;
    static int hTiles = 0x010;

    for (auto xTile=0; xTile < wTiles; xTile++)
    {
        for (auto yTile=0; yTile < hTiles; yTile++)
        {
            //uint16_t tileOffset = (xTile*0x10 + yTile) * 0x10;
            uint16_t tileOffset = (yTile << 8) + (xTile << 4);
            //auto tileOffset = 0;

            for (auto dy=0; dy < 8; dy++)
            {
                auto yPixel = yTile*8 + dy;

                uint8_t rowDataPlane1 = cart->chrData(iChrBlock, tileOffset + dy +     iTable*0x1000);
                uint8_t rowDataPlane2 = cart->chrData(iChrBlock, tileOffset + dy + 8 + iTable*0x1000);

                for (auto dx=0; dx < 8; dx++)
                {
                    auto xPixel = (xTile+1)*8 - dx - 1;
                    //auto xPixel = xTile*8 + dx;
                    uint8_t valuePlane1 = (rowDataPlane1 >> dx) & 0x1;
                    uint8_t valuePlane2 = (rowDataPlane2 >> dx) & 0x1;
                    screenData[xPixel][yPixel] = (valuePlane2 << 1) + valuePlane1;
                }
            }
        }
    }
}


void renderPatternTable(uint8_t (&screenData)[PTABLE_WIDTH][PTABLE_HEIGHT], PatternTableIndex iTable)
{
    for (auto x=0; x < PTABLE_WIDTH; x++)
    {
        for (auto y=0; y < PTABLE_HEIGHT; y++)
        {
            uint8_t* newColor;
            switch (screenData[x][y]) {
                case 0x00:
                    newColor = (uint8_t *)COLOR_BKG;
                    break;
                case 0x01:
                    newColor = (uint8_t *)COLOR_1;
                    break;
                case 0x02:
                    newColor = (uint8_t *)COLOR_2;
                    break;
                case 0x03:
                    newColor = (uint8_t *)COLOR_3;
                    break;
                default:
                    newColor = (uint8_t *)COLOR_KO;
                    break;
            }

            SDL_SetRenderDrawColor(renderer, newColor[0], newColor[1], newColor[2], newColor[3]);

            SDL_Rect rect((iTable*PTABLE_WIDTH + x)*SCALE_FACTOR, y*SCALE_FACTOR, SCALE_FACTOR, SCALE_FACTOR);
            SDL_RenderFillRect(renderer, &rect);
            // cout << "After SDL_RenderFillRect" << endl;
        }
    }

    SDL_RenderPresent(renderer);
}