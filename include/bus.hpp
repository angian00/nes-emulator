#pragma once

#include "cartridge.hpp"
#include "cpu.hpp"
#include "ppu.hpp"

class Bus
{
public:
    static const uint16_t INTERNAL_RAM_SIZE = 0x800;
    Bus();
    Cpu* cpu() { return m_cpu; }
    Ppu* ppu() { return m_ppu; }

    void insertCartridge(Cartridge* cart);
    void reset();

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);
    uint8_t readChr(uint16_t addr);

    
private:
    uint8_t m_internalRam[INTERNAL_RAM_SIZE];

    Cartridge* m_cart;
    Cpu* m_cpu;
    Ppu* m_ppu;
};
