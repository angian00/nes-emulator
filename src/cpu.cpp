#include "cpu.hpp"

#include "bus.hpp"
#include "instruction.hpp"

#include <print>

//#undef PRINT_OPCODES
#define PRINT_OPCODES


void Cpu::reset()
{
    A = 0x00;
    X = 0x00;
    Y = 0x00;
    SP = 0xFD;     // offset to the start of the stack
    PC = 0x0000;
    P = 0x24; // IRQ disabled

    uint16_t lo = m_bus->read(0xFFFC);
    uint16_t hi = m_bus->read(0xFFFD);
    PC = (hi << 8) | lo;
    
    nWaitCycles = 8;
    nProcessedInstr = 0;
}


bool Cpu::hasFlag(FlagIndex flagIndex)
{
    return ((P >> flagIndex) & 0x1) == 1;
}

void Cpu::setFlag(FlagIndex flagIndex, bool value)
{
    uint8_t mask = (0x1 << flagIndex);
    if (value)
        P |= mask;
    else
        P &= ~mask;
}
    
uint8_t Cpu::read(uint16_t addr)
{
    return m_bus->read(addr);
}

void Cpu::write(uint16_t addr, uint8_t value)
{
    m_bus->write(addr, value);
}


void Cpu::pushStack(uint8_t value)
{
    if (SP == 0xFF)
        throw std::runtime_error("stack overflow");

    m_bus->write(STACK_START + (SP++), value);
}

uint8_t Cpu::popStack()
{
    if (SP == 0x00)
        throw std::runtime_error("stack underflow");

    return m_bus->read(STACK_START + (--SP));
}


void Cpu::clock()
{
    //std::println("Cpu::clock()");

    if (nWaitCycles == 0) {
        nProcessedInstr ++;
        auto opcode = read(PC++);

        Instruction& instr = instructionLookupTable[opcode];
        nWaitCycles = instr.nCycles;
#ifdef PRINT_OPCODES
       std::println("[{:04d}] PC: ${:04X} opcode: 0x{:02X} {}", 
            nProcessedInstr, PC-1, opcode, instr.name);
#endif

        m_currAddrMode = instr.addrmode;
        uint8_t extraCycles1 = (this->*instr.addrmode)();
        uint8_t extraCycles2 = (this->*instr.operate)();  
        //nWaitCycles += (extraCycles1 & extraCycles2);
    }

    nWaitCycles--;
}

