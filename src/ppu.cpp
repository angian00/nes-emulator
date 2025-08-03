#include "ppu.hpp"

#include "bus.hpp"
#include "bit_operations.hpp"

#include <print>
#include <cstring>


static const uint16_t START_NAME_TABLES = 0x2000;
static const uint16_t NAME_TABLE_SIZE = 0x400;
static const uint16_t ATTR_TABLE_OFFSET = 0xC0;

    
static uint8_t wTiles = 32;
static uint8_t hTiles = 30;
static const uint8_t iNameTable = 0; //TODO: support multiple nametables
static const uint16_t startNameTable = START_NAME_TABLES + iNameTable * NAME_TABLE_SIZE;

void Ppu::reset()
{
    memset(m_pixels, 0x00, sizeof(m_pixels));
    memset(m_internalRam, 0x00, sizeof(m_internalRam));
    
    m_registers[Register::PPUCTRL] = 0x00;
    m_registers[Register::PPUMASK] = 0x00;
    m_registers[Register::PPUSCROLL] = 0x00;
    m_registers[Register::PPUDATA] = 0x00;

    m_internalRegisterV = 0x00;
    m_internalRegisterT = 0x00;
    m_internalRegisterX = 0x00;
    m_internalRegisterW = 0x00;

    m_dot = 0;
    m_scanline = 0;
    m_frameComplete = false;
    m_oddFrame = false;
}

uint8_t Ppu::readRegister(Register reg)
{
    // side-effects on internal registers
    // see https://www.nesdev.org/wiki/PPU_scrolling#Summary
    if (reg == Register::PPUSTATUS)
            m_internalRegisterW = 0x00;

    return m_registers[reg];
}

void Ppu::writeRegister(Register reg, uint8_t value)
{
    m_registers[reg] = value;

    // side-effects on internal registers
    // see https://www.nesdev.org/wiki/PPU_scrolling#Summary
    if (reg == Register::PPUCTRL)
    {
        assignBits(&m_internalRegisterT, value, 10, 0, 2);
    }
    else if (reg == Register::PPUSCROLL)
    {
        if (m_internalRegisterW == 0x00)
        {
            //first write
            assignBits(&m_internalRegisterT, value, 0, 3, 5);
            m_internalRegisterX = value & 0x07;
            m_internalRegisterW = 0x01;
        }
        else
        {
            //second write
            assignBits(&m_internalRegisterT, value, 12, 0, 3);
            assignBits(&m_internalRegisterT, value, 5, 3, 3);
            assignBits(&m_internalRegisterT, value, 8, 6, 2);
            m_internalRegisterW = 0x00;
        }
    }
    else if (reg == Register::PPUADDR)
    {
        if (m_internalRegisterW == 0x00)
        {
            //first write
            assignBits(&m_internalRegisterT, value, 8, 0, 6);
            m_internalRegisterT &= ~(1 << 14); //clear Z bit            
            m_internalRegisterW = 0x01;
        }
        else
        {
            //second write
            assignBits(&m_internalRegisterT, value, 0, 0, 8);
            m_internalRegisterV = m_internalRegisterT;
            m_internalRegisterW = 0x00;
        }
    }
}


uint8_t Ppu::read(uint16_t addr)
{
    if (addr >= 0x3F00)
    {
        //TODO: palette ram indexes
        return 0x00;
    }

    if (addr >= 0x2000)
    {
        //access bus internal ram
        std::println("read: accessing bus internal ram");
        return m_bus->read(addr - 0x2000);
    }

    //access bus chr rom
    return m_bus->readChr(addr);
    // uint8_t value = m_bus->readChr(addr);

    // if (addr != 0x0000 || value != 0)
    //     std::println("read: accessing bus chr [{:04X}]; value: [{:02X}]", addr, value);
    // return value;
}

void Ppu::write(uint16_t addr, uint8_t value)
{
    if (addr >= 0x3F00)
    {
        //TODO: palette ram indexes
        return;
    }

    if (addr >= 0x2000)
    {
        //access bus internal ram
        m_bus->write(addr - 0x2000, value);
        return;
    }

    //access bus chr rom
    throw std::runtime_error("write access to ROM");
}



void Ppu::clock()
{
    // see https://www.nesdev.org/wiki/PPU_rendering,
    // especially the frame timing diagram
    //
    // ... rendering logic (fetch, shift registers, etc.) ...
    //Fetch a nametable entry from $2000-$2FFF.
    //Fetch the corresponding attribute table entry from $23C0-$2FFF and increment the current VRAM address within the same row.
    //Fetch the low-order byte of an 8x1 pixel sliver of pattern table from $0000-$0FF7 or $1000-$1FF7.
    //Fetch the high-order byte of this sliver from an address 8 bytes higher.
    //Turn the attribute data and the pattern table data into palette indices, and combine them with data from sprite data using priority.

    // pre-render scanline
    if (m_scanline == 261)
    {
        if (m_dot == 1)
        {
            m_registers[Register::PPUSTATUS] = 0x00;
        }
        else if (m_dot == 304)
        {
            m_internalRegisterV = m_internalRegisterT;
        }
    }

    if (m_scanline >= 0 && m_scanline <= 239)
    {
        uint8_t m_dotPhase = m_dot % 8;

        switch (m_dotPhase)
        {
            case 0:
                if (m_dot == 0)
                {
                    //TODO: BG lsbit, skipped on even frames
                }
                else
                {
                    //inc hori(v)
                }
                break;
        
            case 1:
            {
                //NT (first)
                uint16_t ntIndex = currentTileOffset();
                m_ntEntry = read(startNameTable + ntIndex);
                break;
            }
            case 2:
                //NT (second)
                break;

            case 3:
            {
                //AT (first)
                uint16_t attrTableIndex = 0x00; //TODO
                //m_attrEntry = read(startNameTable + ATTR_TABLE_OFFSET + attrTableIndex);
                break;
            }
            case 4:
                //AT (second)
                break;

            case 5:
                //BG lsbits (first)
                m_rowDataPlane1 = read(m_ntEntry + currentFineY());
                break;
            case 6:
                //BG lsbits (second)
                break;

            case 7:
                //BG msbits (first?)
                m_rowDataPlane1 = read(m_ntEntry + currentFineY() + 8);
                break;
        }
    }    

    //post-render scanline
    if (m_scanline == 241 && m_dot == 1)
    {
        m_registers[Register::PPUSTATUS] |= (1<<StatusFlag::VBlank);

        // if (control.generateNMI)
        //     m_bus->cpu()->requestNMI();
    }

    if (m_dot > 0 && m_dot <= 256)
        renderPixel();
    
    if ((m_dot % 8) == 0)
        incrementCoarseX();
    if (m_dot == 256)
        incrementY();

    m_dot++;

    if (m_dot > 340) {
        m_dot = 0;
        m_scanline++;

        if (m_scanline > 261) {
            m_scanline = 0;
            m_frameComplete = true;
        }
    }
}



// void Ppu::testNameTables()
// {
//     uint16_t attrTableIndex =  0x00;

//     static const int iNameTable = 0;
//     uint16_t startNameTable = START_NAME_TABLES + iNameTable * NAME_TABLE_SIZE;

//     std::println("testNameTables()");

//     uint8_t attributes = read(startNameTable + ATTR_TABLE_OFFSET + attrTableIndex);
//     std::println("attributes={:02X}", attributes);
//     for (uint8_t xTile=0; xTile < wTiles; xTile++)
//     {
//         std::println("xTile loop: ({})", xTile);

//         for (uint8_t yTile=0; yTile < hTiles; yTile++)
//         {
//             std::println("xTile, yTile loop: ({}, {})", xTile, yTile);

//             //uint16_t ntEntry = cart->chrData(iChrBlock, startNameTable + yTile*hTiles + xTile);
//             uint16_t ntEntry = read(startNameTable + xTile*wTiles + yTile);
//             //uint8_t ntEntry = 0x00;
//             if (ntEntry != 0x0000)
//                 std::println("testNameTables; xTile={}, yTile={}; ntEntry={:04X}", xTile, yTile, ntEntry);

//             uint16_t ptOffset = ntEntry << 4;

//              //construct tile from pattern table data
//             for (auto dy=0; dy < 8; dy++)
//             {
//                 uint8_t rowDataPlane1 = read(ptOffset + dy);
//                 uint8_t rowDataPlane2 = read(ptOffset + dy + 8);

//                 uint8_t y = 8*yTile + dy;
//                 for (auto dx=0; dx < 8; dx++)
//                 {
//                     uint8_t x = 8*xTile + (8 - dx - 1);
//                     uint8_t valuePlane1 = (rowDataPlane1 >> dx) & 0x1;
//                     uint8_t valuePlane2 = (rowDataPlane2 >> dx) & 0x1;

//                     std::println("setting pixel ({}, {})", x, y);

//                     m_pixels[x*SCREEN_HEIGHT + y] = (valuePlane2 << 1) + valuePlane1;
//                 }
//             }
//         }
//     }
// }

uint16_t Ppu::currentTileOffset()
{
    //CHECK
    uint16_t coarseX = (m_internalRegisterV & 0x1F);
    uint16_t coarseY = (m_internalRegisterV >> 5) & 0x1F;
    return coarseX*wTiles + coarseY;
    //return (m_internalRegisterV & 0xFFF);
}

uint8_t Ppu::currentFineY()
{
    return (m_internalRegisterV >> 12);
}



void Ppu::incrementCoarseX()
{
    // see https://www.nesdev.org/wiki/PPU_scrolling#Wrapping_around
    // The coarse X component of v needs to be incremented when the next tile is reached. Bits 0-4 are incremented, with overflow toggling bit 10. This means that bits 0-4 count from 0 to 31 across a single nametable, and bit 10 selects the current nametable horizontally.

    if ((m_internalRegisterV & 0x001F) == 31)
    {
        m_internalRegisterV &= ~0x001F;
        //m_internalRegisterV ^= 0x0400; // TODO: switch horizontal nametable
    }
    else
    {
        m_internalRegisterV ++;
    }
}

void Ppu::incrementY()
{
    // see https://www.nesdev.org/wiki/PPU_scrolling#Wrapping_around
    // If rendering is enabled, fine Y is incremented at dot 256 of each scanline, overflowing to coarse Y, and finally adjusted to wrap among the nametables vertically.

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
            //m_internalRegisterV ^= 0x0800; // TODO: switch vertical nametable
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
    }
}


void Ppu::renderPixel()
{
    int bit = 0x8000 >> fineX;

    uint8_t pixel_lo = (pattern_shift_low & bit) ? 1 : 0;
    uint8_t pixel_hi = (pattern_shift_high & bit) ? 1 : 0;
    uint8_t pixel = (pixel_hi << 1) | pixel_lo;

    // uint8_t attr_lo = (attribute_shift_low & bit) ? 1 : 0;
    // uint8_t attr_hi = (attribute_shift_high & bit) ? 1 : 0;
    // uint8_t palette = (attr_hi << 1) | attr_lo;

    // uint8_t color = ppuRead(0x3F00 + (palette << 2) + pixel);

    //m_frameBuffer[m_scanline][cycle - 1] = systemPalette[color]
    m_frameBuffer[m_scanline][m_dot - 1] = pixel;
}


void Ppu::dumpFrameBuffer()
{
    std::println("-- Ppu::dumpFrameBuffer");
    for (auto x=0; x < SCREEN_WIDTH; x++)
    {
        for (auto y=0; y < SCREEN_HEIGHT; y++)
        {
            std::print("({:01X})", m_frameBuffer[x*SCREEN_HEIGHT + y]);
        }
        std::println();
    }
    std::println();
}

