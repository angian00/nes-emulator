#include "cpu.hpp"

#include "bus.hpp"
#include "instructions.hpp"

#include <print>



bool isPageBreak(uint16_t addr1, uint16_t addr2)
{
    return ((addr1 & 0xff00) != (addr2 & 0xff00));
}


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
    
    //m_nWaitCycles = 8;
    m_nWaitCycles = 7;
    m_nProcessedInstr = 0;
    m_nTotCycles = 0;
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
    if (SP == 0x00)
        throw std::runtime_error("stack overflow");

    m_bus->write(STACK_START + (SP--), value);
}

uint8_t Cpu::popStack()
{
    if (SP == 0xFF)
        throw std::runtime_error("stack underflow");

    return m_bus->read(STACK_START + (++SP));
}


void Cpu::clock()
{
    //std::println("Cpu::clock()");

    if (m_nWaitCycles == 0) {
        m_nProcessedInstr ++;
        auto opcode = read(PC++);

        Instruction& instr = instructionLookupTable[opcode];
        m_nWaitCycles = instr.nCycles;

        if (m_tracing) {
//            std::println("[{:04d}] PC: ${:04X}  opcode: 0x{:02X} {}   SP: 0x{:02X}   A: 0x{:02X} X: 0x{:02X} Y: 0x{:02X} P: 0x{:02X} ", m_nProcessedInstr, PC-1, opcode, instr.name, SP, A, X, Y, P);
            //C000  4C F5 C5  JMP $C5F5                       A:00 X:00 Y:00 P:24 SP:FD PPU:  0, 21 CYC:7
            std::string opcodeBytes = "";
            std::string args = "";
            uint8_t ppu1 = m_bus->ppu()->peekRegister(Ppu::Register::PPUCTRL);
            uint8_t ppu2 = m_bus->ppu()->peekRegister(Ppu::Register::PPUDATA);
            std::print("{:04X}  {:9s} {} {:27s} ", PC-1, opcodeBytes, instr.name == "???" ? "NOP" : instr.name, args);
            std::print("A:{:02X} X:{:02X} Y:{:02X} P:{:02X} SP:{:02X} ", A, X, Y, P, SP);
            std::println("PPU:{:3d},{:3d} CYC:{:d}", ppu1, ppu2, m_nTotCycles);
        }

        m_currAddrMode = instr.addrmode;
        uint8_t extraCycles1 = (this->*instr.addrmode)();
        uint8_t extraCycles2 = (this->*instr.operate)();  
        m_nWaitCycles += extraCycles1 & extraCycles2;
    }

    m_nWaitCycles--;
    m_nTotCycles++;
}

