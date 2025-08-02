
#include "cartridge.hpp"

#include <print>
#include <cassert>
#include <SDL.h>


static const int PTABLE_WIDTH  = 128;
static const int PTABLE_HEIGHT = 128;
static const int SCALE_FACTOR = 6;


static const int SCREEN_WIDTH  = 340;
static const int SCREEN_HEIGHT = 300;

static const uint16_t PTABLE_SIZE = 0x1000;
static const uint16_t START_NAME_TABLES = 0x2000;
static const uint16_t NAME_TABLE_SIZE = 0x400;
static const uint16_t ATTRIBUTE_TABLE_OFFSET = 0xC0;

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

void loadPatternTable(Cartridge* cart, uint8_t (&pixels)[PTABLE_WIDTH][PTABLE_HEIGHT], PatternTableIndex iPTable);
void renderPatternTable(uint8_t (&pixels)[PTABLE_WIDTH][PTABLE_HEIGHT], PatternTableIndex iPTable);
void loadNameTable(Cartridge* cart, uint8_t (&pixels)[SCREEN_WIDTH][SCREEN_HEIGHT], uint8_t iNameTable);
void renderNameTable(uint8_t (&pixels)[SCREEN_WIDTH][SCREEN_HEIGHT]);

void loadPTableTile(Cartridge* cart, uint8_t iChrBlock, PatternTableIndex iPTable, uint16_t tileOffset, uint8_t* pixels);


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

    if (!initSdl())
    {
        shutdownSdl();
        exit(1);
    }

    auto showPatternTable = false;
    if (showPatternTable)
    {
        uint8_t pixels[PTABLE_WIDTH][PTABLE_HEIGHT] = {};
    
        loadPatternTable(cart, pixels, PatternTableIndex::Left);
        renderPatternTable(pixels, PatternTableIndex::Left);
        loadPatternTable(cart, pixels, PatternTableIndex::Right);
        renderPatternTable(pixels, PatternTableIndex::Right);
    }
    else 
    {
        uint8_t pixels[SCREEN_WIDTH][SCREEN_HEIGHT] = {};
    
        loadNameTable(cart, pixels, 0);
        renderNameTable(pixels);
    }

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

    auto PTABLE_SCREEN_WIDTH = 2 * PTABLE_WIDTH * SCALE_FACTOR;
    auto PTABLE_SCREEN_HEIGHT = PTABLE_HEIGHT * SCALE_FACTOR;

    window = SDL_CreateWindow("NES CHR Viewer by AnGian",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          PTABLE_SCREEN_WIDTH, PTABLE_SCREEN_HEIGHT,
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



void loadPatternTable(Cartridge* cart, uint8_t (&pixels)[PTABLE_WIDTH][PTABLE_HEIGHT], 
        PatternTableIndex iPTable)
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

            uint8_t tilePixels[8*8];
            loadPTableTile(cart, iChrBlock, iPTable, tileOffset, tilePixels);
            for (auto dy=0; dy < 8; dy++)
            {
                auto yPixel = yTile*8 + dy;
                for (auto dx=0; dx < 8; dx++)
                {
                    auto xPixel = xTile*8 + dx;
                    pixels[xPixel][yPixel] = tilePixels[dx*8 + dy];
                }
            }
        }
    }
}


void loadPTableTile(Cartridge* cart, uint8_t iChrBlock, PatternTableIndex iPTable, uint16_t tileOffset, uint8_t* pixels)
{
    for (auto y=0; y < 8; y++)
    {
        uint8_t rowDataPlane1 = cart->chrData(iChrBlock, tileOffset + y +     iPTable*PTABLE_SIZE);
        uint8_t rowDataPlane2 = cart->chrData(iChrBlock, tileOffset + y + 8 + iPTable*PTABLE_SIZE);

        for (auto x=0; x < 8; x++)
        {
            uint8_t valuePlane1 = (rowDataPlane1 >> x) & 0x1;
            uint8_t valuePlane2 = (rowDataPlane2 >> x) & 0x1;
            pixels[(8 - x - 1)*8 + y] = (valuePlane2 << 1) + valuePlane1;
        }
    }
}

void renderPatternTable(uint8_t (&pixels)[PTABLE_WIDTH][PTABLE_HEIGHT], PatternTableIndex iPTable)
{
    for (auto x=0; x < PTABLE_WIDTH; x++)
    {
        for (auto y=0; y < PTABLE_HEIGHT; y++)
        {
            uint8_t* newColor;
            switch (pixels[x][y]) {
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

            SDL_Rect rect((iPTable*PTABLE_WIDTH + x)*SCALE_FACTOR, y*SCALE_FACTOR, SCALE_FACTOR, SCALE_FACTOR);
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);
}


void loadNameTable(Cartridge* cart, uint8_t (&pixels)[SCREEN_WIDTH][SCREEN_HEIGHT], uint8_t iNameTable)
{
    assert(iNameTable < 4);

    static int iChrBlock = 0;
    static PatternTableIndex iPTable = PatternTableIndex::Right;
    uint16_t attrTableIndex =  0x00;

    uint16_t startNameTable = START_NAME_TABLES + iNameTable * NAME_TABLE_SIZE;
    
    static uint8_t wTiles = 32;
    static uint8_t hTiles = 30;

    uint8_t attributes = cart->chrData(iChrBlock, startNameTable + ATTRIBUTE_TABLE_OFFSET + attrTableIndex);
    for (uint8_t xTile; xTile < wTiles; xTile++)
    {
        for (uint8_t yTile; yTile < hTiles; yTile++)
        {
            //uint16_t nametableEntry = cart->chrData(iChrBlock, startNameTable + yTile*hTiles + xTile);
            uint16_t nametableEntry = cart->chrData(iChrBlock, startNameTable + xTile*wTiles + yTile);
            //uint8_t nametableEntry = 0x00;
            if (nametableEntry != 0)
                std::println("loadNameTable; xTile={}, yTile={}; nametableEntry={:04X}", 
                xTile, yTile, nametableEntry);

            uint16_t pTableOffset = nametableEntry << 4;

            uint8_t tilePixels[8*8];
            loadPTableTile(cart, iChrBlock, iPTable, nametableEntry, tilePixels);

            for (auto dy=0; dy < 8; dy++)
            {
                auto yPixel = yTile*8 + dy;
                for (auto dx=0; dx < 8; dx++)
                {
                    auto xPixel = xTile*8 + dx;
                    pixels[xPixel][yPixel] = tilePixels[dx*8 + dy];
                }
            }
        }
    }
}


void renderNameTable(uint8_t (&pixels)[SCREEN_WIDTH][SCREEN_HEIGHT])
{
    for (auto x=0; x < SCREEN_WIDTH; x++)
    {
        for (auto y=0; y < SCREEN_HEIGHT; y++)
        {
            uint8_t* newColor;
            switch (pixels[x][y]) {
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

            SDL_Rect rect(x*SCALE_FACTOR, y*SCALE_FACTOR, SCALE_FACTOR, SCALE_FACTOR);
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);
}
