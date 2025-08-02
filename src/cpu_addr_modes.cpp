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
    targetAddress = A;
    return 0x00;
}
uint8_t Cpu::AddrIMM()
{
    targetAddress = PC++;
    return 0x00;
}
uint8_t Cpu::AddrZP0()
{
    targetAddress = read(PC++);
    return 0x00;
}
uint8_t Cpu::AddrZPX()
{
    targetAddress = read(PC++) + X;
    return 0x01; //CHECK
}
uint8_t Cpu::AddrZPY()
{
    targetAddress = read(PC++) + Y;
    return 0x00;
}
uint8_t Cpu::AddrREL()
{
    uint16_t rel = read(PC++);
    if (rel & 0x80)
        //negative offset
        rel |= 0xFF00;

    targetAddress = PC + rel;
    return 0x00;  //CHECK page cross
}

uint8_t Cpu::AddrABS()
{
    uint16_t lo = read(PC++);
    uint16_t hi = read(PC++);
    targetAddress = (hi << 8) | lo;

    return 0x00;
}

uint8_t Cpu::AddrABX()
{
    uint16_t lo = read(PC++);
    uint16_t hi = read(PC++);

    targetAddress = ((hi << 8) | lo) + X;
    if ((targetAddress & 0xff00) != (hi << 8))
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

    targetAddress = ((hi << 8) | lo) + Y;
    if ((targetAddress & 0xff00) != (hi << 8))
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

    uint16_t lo2 = read(addr1++);
    uint16_t hi2 = read(addr1);
    targetAddress = ((hi2 << 8) | lo2);

    //TODO: check additional clock cycle

    return 0x00;
}
uint8_t Cpu::AddrIZX()
{
    uint8_t ll = read(PC++);
    uint16_t addr1 = X + ll;

    uint16_t lo2 = read(addr1 & 0xff);
    addr1 ++;
    uint16_t hi2 = read(addr1 & 0xff);
    targetAddress = ((hi2 << 8) | lo2);

    return 0x00;
}
uint8_t Cpu::AddrIZY()
{
    uint8_t ll = read(PC++);
    uint16_t addr1 = ll;

    uint16_t lo2 = read(addr1++);
    uint16_t hi2 = read(addr1);
    targetAddress = ((hi2 << 8) | lo2) + Y;
    //TODO: check additional clock cycle
    
    return 0x00;
}

