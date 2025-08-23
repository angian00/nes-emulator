#include "bus.hpp"

#include <print>


// ---- memory layout ----
// see https://www.nesdev.org/wiki/CPU_memory_map
//
// address range  size
// $0000–$07FF    $0800    2KB interna RAM; of which...
// ...
// $0000-$00FF    $0100    'Zero Page'
// $0100-$01FF    $0100    system stack
// ...
// $0800–$0FFF    $0800    mirror of $0000–$07FF
// $1000–$17FF    $0800    mirror of $0000–$07FF
// $1800–$1FFF    $0800    mirror of $0000–$07FF
// $2000–$2007    $0008    NES PPU registers
// $2008–$3FFF    $1FF8    mirrors of $2000–$2007 (repeats every 8 bytes)
// $4000–$4017    $0018    NES APU and I/O registers
// $4018–$401F    $0008    APU and I/O functionality that is normally disabled. See CPU Test Mod
// $4020–$FFFF    $BFE0    Unmapped. Available for cartridge use; of which...
// ...
// $6000–$7FFF    $2000    Usually cartridge RAM, when present.
// $8000–$FFFF    $8000    Usually cartridge ROM and mapper registers.
// ...
// $FFFA-$FFFB    $0001    non-maskable interrupt handler (NMI vector)
// $FFFC-$FFFD    $0001    power on reset location (reset vector)
// $FFFE-$FFFF    $0001    BRK/interrupt request handler (IRQ/BRK vector)


uint16_t mapInternalRam(uint16_t addr);
uint16_t mapCartridgeRom(uint16_t addr);
Ppu::Register mapPPURegister(uint16_t addr);


Bus::Bus()
{
    m_cpu = new Cpu();
    m_cpu->connect(this);

    m_ppu = new Ppu();
    m_ppu->connect(this);
}

void Bus::insertCartridge(Cartridge* cart)
{
    m_cart = cart;
}

void Bus::reset()
{
    m_cpu->reset();
    m_ppu->reset();
}


uint8_t Bus::read(uint16_t addr)
{
    if (addr >= 0x8000)
    {
        static const uint8_t iPrgBlock = 0; //TODO: support prg block switching
        addr = mapCartridgeRom(addr);
        return m_cart->prgData(iPrgBlock, addr);
    }

    if (addr >= 0x6000)
        throw std::runtime_error(std::format("access to cartridge RAM area unimplemented; addr=0x{:02X}", addr));

    if (addr >= 0x4000)
    {
        //TODO: NES APU and I/O registers
        return  0x00;
    }

    if (addr >= 0x2000)
    {
        //NES PPU registers
        return m_ppu->readRegister(mapPPURegister(addr));
    }

    addr = mapInternalRam(addr);
    return m_internalRam[addr];
}


void Bus::write(uint16_t addr, uint8_t value)
{
    if (addr >= 0x8000)
    {
        throw std::runtime_error(std::format("Trying to write to read-only memory?; addr=0x{:04X}", addr));
    }

    if (addr >= 0x6000)
        throw std::runtime_error(std::format("access to cartridge RAM area unimplemented; addr=0x{:04X}", addr));

    if (addr >= 0x4000)
    {
        //TODO: NES APU and I/O registers
        return;
    }

    if (addr >= 0x2000)
    {
        //NES PPU registers
        m_ppu->writeRegister(mapPPURegister(addr), value);
        return;
    }

    if (addr >= 0x0100 && addr < 0x0200)
    {
        std::println("Writing on stack; addr=${:04X}, value=${:02X}", addr, value);
    }

    addr = mapInternalRam(addr);
    m_internalRam[addr] = value;
}


uint8_t Bus::readChr(uint16_t addr)
{
    static const uint8_t iChrBlock = 0; //TODO: support chr block switching
    return m_cart->chrData(iChrBlock, addr);

    //uint8_t value = m_cart->chrData(iChrBlock, addr);
    //std::println("Bus::readChr(0x{:04X})=[0x{:02X}]", addr, value);
    //return value;
}



uint16_t mapInternalRam(uint16_t addr)
{
    return (addr % 0x0800);
}

Ppu::Register mapPPURegister(uint16_t addr)
{
    return ((Ppu::Register)(addr % 0x0008));
}


uint16_t mapCartridgeRom(uint16_t addr)
{
    // // Mapper 0 (NROM) mirroring
    // if (addr >= 0xC000)
    //     return addr - 0xC000;
    // else
    //     return addr - 0x8000;

    if (addr >= 0x8000)
        addr = (addr - 0x8000) & 0x3FFF;

    return addr;
}
