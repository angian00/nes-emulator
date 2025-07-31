#pragma once

#include "cpu.hpp"
#include "cartridge.hpp"

class Bus
{
public:
    Bus();
    Cpu* cpu() { return m_cpu; }

    void insertCartridge(Cartridge* cart);
    void reset();

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);

private:

    //uint8_t ram[RAM_SIZE];

    Cartridge* m_cart;
    Cpu* m_cpu;
    //PPU* ppu;
};
