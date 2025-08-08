#include "cpu.hpp"

#include "bus.hpp"
#include "instruction.hpp"

#include <print>


//---- addressing mode implementation ----
// see https://www.nesdev.org/obelisk-6502-guide/addressing.html

uint8_t Cpu::AddrIMP()
{
    return 0x00;
}
uint8_t Cpu::AddrACC()
{
    //CHECK
    m_targetAddress = A;
    return 0x00;
}
uint8_t Cpu::AddrIMM()
{
    m_targetAddress = PC++;
    return 0x00;
}
uint8_t Cpu::AddrZP0()
{
    m_targetAddress = read(PC++);
    return 0x00;
}
uint8_t Cpu::AddrZPX()
{
    m_targetAddress = read(PC++) + X;
    return 0x01; //CHECK additional clock cycle
}
uint8_t Cpu::AddrZPY()
{
    m_targetAddress = read(PC++) + Y;
    return 0x00;
}
uint8_t Cpu::AddrREL()
{
    uint16_t rel = read(PC++);
    if (rel & 0x80)
        //negative offset
        rel |= 0xFF00;

    m_targetAddress = PC + rel;

    return 0x00;
}

uint8_t Cpu::AddrABS()
{
    uint16_t lo = read(PC++);
    uint16_t hi = read(PC++);
    m_targetAddress = (hi << 8) | lo;

    return 0x00;
}

uint8_t Cpu::AddrABX()
{
    uint16_t lo = read(PC++);
    uint16_t hi = read(PC++);

    m_targetAddress = ((hi << 8) | lo) + X;
    if ((m_targetAddress & 0xff00) != (hi << 8))
    {
        //page cross consumes a clock cycle
        return 0x01;
    }

    return 0x00;
}
uint8_t Cpu::AddrABY()
{
    uint16_t lo = read(PC++);
    uint16_t hi = read(PC++);

    m_targetAddress = ((hi << 8) | lo) + Y;
    if ((m_targetAddress & 0xff00) != (hi << 8))
    {
        //page cross consumes a clock cycle
        return 0x01;
    }

    return 0x00;
}
uint8_t Cpu::AddrIND()
{
    uint16_t lo1 = read(PC++);
    uint16_t hi1 = read(PC++);
    uint16_t addr1 = ((hi1 << 8) | lo1);

    uint16_t lo2 = read(addr1);
    addr1 ++;
    //replicate a know 6502 CPU bug
    if ((addr1 & 0xFF) == 0x00)
        addr1 -= 0x100;
    //

    uint16_t hi2 = read(addr1);
    m_targetAddress = ((hi2 << 8) | lo2);

    //m_targetAddress = ((hi1 << 8) | lo1);

    return 0x00;
}
uint8_t Cpu::AddrIZX()
{
    uint8_t ll = read(PC++);
    uint16_t addr1 = X + ll;

    uint16_t lo2 = read(addr1 & 0xff);
    addr1 ++;
    uint16_t hi2 = read(addr1 & 0xff);
    m_targetAddress = ((hi2 << 8) | lo2);

    return 0x00;
}
uint8_t Cpu::AddrIZY()
{
    uint8_t ll = read(PC++);
    uint16_t addr1 = ll;

    uint16_t lo2 = read(addr1++);
    uint16_t hi2 = read(addr1 % 256);
    m_targetAddress = ((hi2 << 8) | lo2) + Y;

    
    return (isPageBreak(hi2 << 8, m_targetAddress) ? 0x01 : 0x00); //TODO: differentiate per STA vs LDA
}

