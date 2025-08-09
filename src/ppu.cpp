#include "ppu.hpp"

#include "bus.hpp"
#include "bit_operations.hpp"

#include <print>
#include <cstring>


static const uint16_t START_NAME_TABLES = 0x2000;
static const uint16_t NAME_TABLE_SIZE = 0x400;
static const uint16_t ATTR_TABLE_OFFSET = 0xC0;
    static const uint16_t START_PALETTE_RAM = 0x3F00;

    
static uint8_t N_TILES_X = 32;
static uint8_t N_TILES_Y = 30;


void Ppu::reset()
{
    memset(m_frameBuffer, 0x00, sizeof(m_frameBuffer));
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
    m_scanline = 261;
    // m_scanline = 0;
    m_frameComplete = false;
    m_oddFrame = false;
}


uint8_t Ppu::read(uint16_t addr)
{
    if (addr >= 0x4000)
    {
        throw std::runtime_error(std::format("access to PPU memory out of bounds; addr=0x{:02X}", addr));
    }

    if (addr >= 0x3F00)
    {
        //access palette ram
        return m_paletteRam[(addr - 0x3F00) % PALETTE_RAM_SIZE];
    }

    if (addr >= 0x2000)
    {
        //access bus internal ram
        //std::println("ppu read: accessing bus internal ram");
        return m_bus->read(addr - 0x2000);
    }

    //access bus chr rom
    return m_bus->readChr(addr);
}

void Ppu::write(uint16_t addr, uint8_t value)
{
    if (addr >= 0x4000)
    {
        throw std::runtime_error(std::format("access to PPU memory out of bounds; addr=0x{:02X}", addr));
    }

    if (addr >= 0x3F00)
    {
        //access palette ram
        std::println("write access to palette RAM; addr={:04X}, value={:02X}", addr, value);
        //TODO: peculiar behaviour where palette index is shared between background and sprites
        m_paletteRam[(addr - 0x3F00) % PALETTE_RAM_SIZE] = value;
        return;
    }

    if (addr >= 0x2000)
    {
        //access bus internal ram
        m_bus->write(addr - 0x2000, value);
        return;
    }

    //access bus chr rom
    throw std::runtime_error("invalid write access to ROM");
}

bool Ppu::isPaletteAddress(uint16_t addr)
{
    return ( (addr >= 0x3F00) && (addr < 0x4000) );

}

uint8_t Ppu::readRegister(Register reg)
{
    return 0x00; //TODO
}

void Ppu::writeRegister(Register reg, uint8_t value)
{
    //TODO;
}


void Ppu::clock()
{
    testNameTables();
    m_frameComplete = true;
}


void Ppu::fillDummyNameTable()
{
    static const int iNameTable = 0;
    uint16_t startNameTable = START_NAME_TABLES + iNameTable * NAME_TABLE_SIZE;
    
    uint8_t ntEntry = 0x00;
    for (uint8_t yTile=0; yTile < N_TILES_Y; yTile++)
    {
        for (uint8_t xTile=0; xTile < N_TILES_X; xTile++)
        {
            ntEntry = yTile * N_TILES_X + xTile;
            //ntEntry = xTile * N_TILES_Y + yTile;
            write(startNameTable + yTile*N_TILES_X + xTile, ntEntry);
        }
    }


    uint16_t attrTableIndex =  0x00;
    
    //checkerboard pattern for palette switching
    for (int i = 0; i < 64; i++) {
        uint8_t attrEntry = ( (i % 4) << 0 ) | ( ((i / 4) % 4) << 2 );
        //uint8_t attrEntry = 0b00011011;
        write(startNameTable + ATTR_TABLE_OFFSET + attrTableIndex, attrEntry);
    }


    uint8_t dummyPalette[32] = {
        0x0F, 0x1F, 0x2F, 0x3F,  // background palette 0
        0x0E, 0x1E, 0x2E, 0x3E,  // background palette 1
        0x0D, 0x1D, 0x2D, 0x3D,  // background palette 2
        0x0C, 0x1C, 0x2C, 0x3C,  // background palette 3
        0x0B, 0x1B, 0x2B, 0x3B,  // sprite palette 0
        0x0A, 0x1A, 0x2A, 0x3A,  // sprite palette 1
        0x09, 0x19, 0x29, 0x39,  // sprite palette 2
        0x08, 0x18, 0x28, 0x38,  // sprite palette 3
    };
    for (int i = 0; i < 32; i++) {
        write(START_PALETTE_RAM, dummyPalette[i]);
    }
}


void Ppu::testNameTables()
{
    uint16_t attrTableIndex =  0x00;

    static const int iNameTable = 0;
    uint16_t startNameTable = START_NAME_TABLES + iNameTable * NAME_TABLE_SIZE;

    std::println("testNameTables()");

    for (uint8_t yTile=0; yTile < N_TILES_Y; yTile++)
    {
        //std::println("xTile loop: ({})", xTile);
        for (uint8_t xTile=0; xTile < N_TILES_X; xTile++)
        {
            //std::println("xTile, yTile loop: ({}, {})", xTile, yTile);
            uint8_t ntEntry = read(startNameTable + yTile*N_TILES_X + xTile);
            //if (ntEntry != 0x0000 && ntEntry != 0x00CD)
            //    std::println("testNameTables; xTile={}, yTile={}; ntEntry={:04X}", xTile, yTile, ntEntry);

            //uint16_t ptOffset = ntEntry << 4; //"left" pattern table
            uint16_t ptOffset = 0x1000 + (ntEntry << 4); //"right" pattern table
            
            uint8_t attrEntry = read(startNameTable + ATTR_TABLE_OFFSET +  (yTile / 4) * 8 + (xTile / 4));
            std::println("attrEntry={:02X}", attrEntry);
            int attrShift = ((yTile % 4) / 2) * 2 + ((xTile % 4) / 2);
            uint8_t paletteIndex = (attrEntry  >> (attrShift  * 2)) & 0x03;

            for (auto dy=0; dy < 8; dy++)
            {
                uint8_t rowDataPlane1 = read(ptOffset + dy);
                uint8_t rowDataPlane2 = read(ptOffset + dy + 8);

                uint8_t y = 8*yTile + dy;
                for (auto dx=0; dx < 8; dx++)
                {
                    uint8_t x = 8*xTile + (8 - dx - 1);
                    uint8_t valuePlane1 = (rowDataPlane1 >> dx) & 0x1;
                    uint8_t valuePlane2 = (rowDataPlane2 >> dx) & 0x1;

                    //std::println("setting pixel ({}, {})", x, y);

                    uint8_t pixel = (valuePlane2 << 1) + valuePlane1;
                    //m_frameBuffer[x*SCREEN_HEIGHT + y] = pixel * 10; //rough "palette indexing"
                    m_frameBuffer[x*SCREEN_HEIGHT + y] = paletteIndex*4 + pixel;
                    
                }
            }
        }
    }

    m_frameComplete = true;
}


void Ppu::dumpFrameBuffer()
{
    std::println("-- Ppu::dumpFrameBuffer");
    for (auto x=0; x < SCREEN_WIDTH; x++)
    {
        for (auto y=0; y < SCREEN_HEIGHT; y++)
        {
            std::print("({:01X})", m_frameBuffer[SCREEN_HEIGHT*x + y]);
        }
        std::println();
    }
    std::println();
}

