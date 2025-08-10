#include "ppu.hpp"

#include "bus.hpp"
#include "bit_operations.hpp"

#include <print>
#include <cassert>

static const uint16_t START_NAME_TABLES = 0x2000;
static const uint16_t NAME_TABLE_SIZE = 0x400;
static const uint16_t ATTR_TABLE_OFFSET = 0x03C0;
static const uint16_t START_PALETTE_RAM = 0x3F00;
    
static uint8_t N_TILES_X = 32;
static uint8_t N_TILES_Y = 30;


void Ppu::fetchAndRender()
{
    // see https://www.nesdev.org/wiki/PPU_rendering,
    // especially the frame timing diagram

    std::println("fetchAndRender; dot={}, scanline={}", m_dot, m_scanline);
    static const int iNameTable = 0;
    static const uint16_t startNameTable = START_NAME_TABLES + iNameTable * NAME_TABLE_SIZE;

    uint8_t yTile = m_scanline / 8;
    uint8_t xTile = m_dot / 8;
    uint8_t dy = m_scanline % 8;

    if ((m_dot % 8) == 0) {
        //std::println("Reading tile {}, {}", xTile, yTile);
        uint8_t ntEntry = read(startNameTable + yTile*N_TILES_X + xTile);
        uint16_t rowDataPlane1 = read(0x1000 + (ntEntry << 4) + dy);
        assignBits(&m_patternShiftLo, rowDataPlane1, 0, 0, 8);
        uint16_t rowDataPlane2 = read(0x1000 + (ntEntry << 4) + dy + 8);
        assignBits(&m_patternShiftHi, rowDataPlane2, 0, 0, 8);

        uint8_t attrEntry = read(startNameTable + ATTR_TABLE_OFFSET + (yTile / 4) * 8 + (xTile / 4));
        int attrShift = ((yTile % 4) / 2) * 2 + ((xTile % 4) / 2);
        m_paletteIndex = (attrEntry  >> (attrShift  * 2)) & 0x03;

    }

    renderPixel(m_dot % 8);
    //updateShiftRegisters();


    //std::println("xTile={}, currentCoarseX={}", xTile, currentCoarseX());
    assert(xTile == currentCoarseX());
    assert(yTile == currentCoarseY());
    assert(dy == currentFineY());


    m_dot ++;
    if (m_dot > 255)
    {
        m_dot = 0;
        incrementY();

        m_scanline ++;
        if (m_scanline > 239)
        {
            m_scanline = 0;
            m_frameComplete = true;
        }
    }
    
    if ((m_dot % 8) == 0) {
        incrementCoarseX();
    }

}


uint8_t Ppu::currentFineY()
{
    return (m_internalRegisterV >> 12);
}

uint8_t Ppu::currentCoarseX()
{
    return m_internalRegisterV & 0b11111;
}

uint8_t Ppu::currentCoarseY()
{
    return (m_internalRegisterV & 0b1111100000) >> 5;
}

void Ppu::incrementCoarseX()
{
    // see https://www.nesdev.org/wiki/PPU_scrolling#Wrapping_around
    // The coarse X component of v needs to be incremented when the next tile is reached. Bits 0-4 are incremented, with overflow toggling bit 10. This means that bits 0-4 count from 0 to 31 across a single nametable, and bit 10 selects the current nametable horizontally.

    if ((m_internalRegisterV & 0x001F) == 31)
    {
        m_internalRegisterV &= ~0x001F; //restart from 0
        m_internalRegisterV ^= 0x0400; // switch horizontal nametable
    }
    else
    {
        m_internalRegisterV ++;
    }
}

void Ppu::incrementY()
{
    // see https://www.nesdev.org/wiki/PPU_scrolling#Wrapping_around
    // If rendering is enabled, fine Y is incremented at dot 256 of each scanline, 
    // overflowing to coarse Y, and finally adjusted to wrap among the nametables vertically.

    // Bits 12-14 are fine Y. Bits 5-9 are coarse Y. Bit 11 selects the vertical nametable.

    if ((m_internalRegisterV & 0x7000) != 0x7000)
    {   
        // if fine Y < 7, increment fine Y
        m_internalRegisterV += 0x1000;
    }
    else
    {
        m_internalRegisterV &= ~0x7000; // fine Y = 0
        // let y = coarse Y
        uint16_t y = (m_internalRegisterV & 0x03E0) >> 5;
        if (y == 29)
        {
            y = 0;
            m_internalRegisterV ^= 0x0800; // TODO: switch vertical nametable
        }
        else if (y == 31)
        {
            // coarse Y = 0, nametable not switched
            y = 0;
        }
        else
        {
            // increment coarse Y
            y += 1;
        }
        m_internalRegisterV = (m_internalRegisterV & ~0x03E0) | (y << 5);
        //assignBits(&m_internalRegisterV, y, 0, 5, 5);
    }
    
}


void Ppu::renderPixel(uint8_t dx)
{
    uint8_t pixelHi = (m_patternShiftHi >> (7 - dx)) & 0x1;
    uint8_t pixelLo = (m_patternShiftLo >> (7 - dx)) & 0x1;
    uint8_t pixel = (pixelHi << 1) | pixelLo;

    // 4bit0
    // -----
    // SAAPP
    // |||||
    // |||++- Pixel value from tile pattern data
    // |++--- Palette number from attributes
    // +----- Background/Sprite select

    //uint8_t colorIndex = read(START_PALETTE_RAM + (palette << 2) + pixel); //KO
    
    //uint8_t colorIndex = 1; // all white
    //uint8_t colorIndex = ((m_dot-1) % 16) < 8 ? 1 : 10;  // vertical stripes
    //uint8_t colorIndex = ((m_scanline) % 16) < 8 ? 1 : 10; // horizonatal stripes

    //int attrShift = ((yTile % 4) / 2) * 2 + ((xTile % 4) / 2);
    //uint8_t paletteIndex = (attrEntry  >> (attrShift  * 2)) & 0x03;
    //uint8_t paletteIndex = attr & 0x03; //TODO
    //uint8_t paletteIndex = 0x00; //TODO: check attr processing

    uint8_t colorIndex = read(START_PALETTE_RAM + m_paletteIndex + pixel);

    long int offset = SCREEN_HEIGHT * m_dot + m_scanline;
    //std::println("renderPixel({}, {}); offset={}", m_dot, m_scanline, offset);
    //std::println("renderPixel({}, {}); pixel={:02X}, colorIndex={}", m_dot, m_scanline, pixel, colorIndex);
    assert(offset >= 0 && offset < SCREEN_HEIGHT*SCREEN_WIDTH);
    m_frameBuffer[offset] = colorIndex;
}


void Ppu::updateShiftRegisters()
{
    m_patternShiftHi <<= 1;
    m_patternShiftLo <<= 1;
    m_attrShiftHi <<= 1;
    m_attrShiftLo <<= 1;
}
