#pragma once

#include "cpu.hpp"
#include "cartridge.hpp"

class Bus
{
public:
    static const uint16_t INTERNAL_RAM_SIZE = 0x800;
    Bus();
    Cpu* cpu() { return m_cpu; }

    void insertCartridge(Cartridge* cart);
    void reset();

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);

private:

    uint8_t internalRam[INTERNAL_RAM_SIZE];

    Cartridge* m_cart;
    Cpu* m_cpu;
    //PPU* ppu;
};
