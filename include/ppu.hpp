#pragma once

#include <cstdint>

class Bus;



class Ppu
{
public:
    enum Register {
        PPUCTRL,
        PPUMASK,
        PPUSTATUS,
        OAMADDR,
        OAMDATA,
        PPUSCROLL,
        PPUADDR,
        PPUDATA,
        OAMDMA,

        N_REGISTERS
    };

    enum StatusFlag {
        SpriteOverflow = 5,
        SpriteZeroHit,
        VBlank
    };

    static const int SCREEN_WIDTH  = 256;
    static const int SCREEN_HEIGHT = 240;
    static const uint16_t INTERNAL_RAM_SIZE = 0x800;
    static const uint16_t PALETTE_RAM_SIZE = 0x20;

    Ppu() {  };
    uint16_t dot() { return m_dot; }
    uint16_t scanline() { return m_scanline; }
    const uint8_t *frameBuffer() { return m_frameBuffer; }
    uint8_t readRegister(Register reg);
    void writeRegister(Register reg, uint8_t value);

    void connect(Bus* bus) { m_bus = bus; }
    void reset();
    void clock();
    bool isFrameComplete() { return m_frameComplete; }
    void clearFrameComplete() { m_frameComplete = false; }

    void testNameTables();
    void fillDummyNameTable();
    void dumpFrameBuffer();

private:
    Bus* m_bus;
    
    uint8_t m_registers[Register::N_REGISTERS] = {}; //"actually PPUSTATUS is usually +0+x xxxx at powerup"
    uint16_t m_internalRegisterV = 0x00;
    uint16_t m_internalRegisterT = 0x00;
    uint8_t  m_internalRegisterX = 0x00;
    uint8_t  m_internalRegisterW = 0x00;

    uint8_t m_frameBuffer[SCREEN_WIDTH*SCREEN_HEIGHT] = {};

    uint8_t m_internalRam[INTERNAL_RAM_SIZE] = {};
    uint8_t m_paletteRam[PALETTE_RAM_SIZE] = {};

    uint16_t m_scanline;
    uint16_t m_dot;
    bool m_frameComplete;
    bool m_oddFrame = false;
    
    //shift registers
    uint16_t m_patternShiftHi;
    uint16_t m_patternShiftLo;
    uint16_t m_attrShiftHi;
    uint16_t m_attrShiftLo;

    //internal latches?
    uint8_t m_ntEntry;
    uint8_t m_attrEntry;
    uint8_t m_ppuDataBuffer;

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);
    bool isPaletteAddress(uint16_t addr);
    
    uint16_t ntDataOffset();
    uint8_t currentFineY();
    void incrementCoarseX();
    void incrementY();

    void renderPixel();
    void updateShiftRegisters();
};
