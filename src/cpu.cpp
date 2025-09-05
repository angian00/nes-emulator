#include "cpu.hpp"

#include "bus.hpp"
#include "instructions.hpp"

#include <print>


bool isPageBreak(uint16_t addr1, uint16_t addr2)
{
    return ((addr1 & 0xff00) != (addr2 & 0xff00));
}


void Cpu::reset(bool isAutoTest)
{
    A = 0x00;
    X = 0x00;
    Y = 0x00;
    SP = 0xFD;     // offset to the start of the stack

    P = 0x24; // IRQ disabled

    m_nmiPending = false;

    if (isAutoTest)
        PC = 0xC000;
    else
    {
        uint16_t lo = m_bus->read(0xFFFC);
        uint16_t hi = m_bus->read(0xFFFD);
        PC = (hi << 8) | lo;
    }

    //m_nWaitCycles = 8;
    m_nWaitCycles = 7;
    m_nProcessedInstr = 0;
    m_nTotCycles = 0;

    m_oamState = OAMState::INACTIVE;
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
    if (addr == 0x4014)
    {
        std::println("writing to OAMDMA; value=${:02X}", value);
        startOAMDMA(value << 8);
        return;
    }
        
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


void Cpu::requestNMI()
{
    m_nmiPending = true;
}

void Cpu::executeNMI()
{
    std::println("NMI occurred");

    m_nmiPending = false;
    pushStack(PC >> 8);
    pushStack(PC & 0xFF);
    pushStack(P & ~0x10);

    P |= 0x04;
    PC = read(0xFFFA) + (read(0xFFFB) << 8);
}


void Cpu::clock()
{
    if (m_oamState == OAMState::READ)
    {
        m_currOAMValue = read(m_nextOAMAddr++);
        m_oamState = OAMState::WRITE;
        return;
    }
    
    if (m_oamState == OAMState::WRITE)
    {
        //write on OAMDATA PPU register
        m_bus->write(0x2004, m_currOAMValue);

        m_nOAMPerformed ++;
        if (m_nOAMPerformed == N_OAMDMA_VALUES)
            m_oamState = OAMState::INACTIVE;
        else
            m_oamState = OAMState::READ;
        return;
    }

    //std::println("Cpu::clock()");
    if (m_nWaitCycles == 0) {
        if (m_nmiPending)
            executeNMI();
        
        m_nProcessedInstr ++;
        auto startPC = PC;
        auto opcode = read(PC++);

        Instruction& instr = instructionLookupTable[opcode];
        m_nWaitCycles = instr.nCycles;

        if (m_tracing) {
            logInstruction(startPC);
        }

        m_currAddrMode = instr.addrmode;
        uint8_t extraCycles1 = (this->*instr.addrmode)();
        uint8_t extraCycles2 = (this->*instr.operate)();  
        m_nWaitCycles += extraCycles1 & extraCycles2;
    }

    m_nWaitCycles--;
    m_nTotCycles++;
}

void Cpu::startOAMDMA(uint16_t startAddr)
{
    m_nextOAMAddr = startAddr;
    m_oamState = OAMState::READ;
    m_nOAMPerformed = 0;
}

void Cpu::logInstruction(uint16_t pc)
{
        auto opcode = read(pc);
        Instruction& instr = instructionLookupTable[opcode];

        //C000  4C F5 C5  JMP $C5F5                       A:00 X:00 Y:00 P:24 SP:FD PPU:  0, 21 CYC:7
        std::string opcodeBytes;
        if (instr.nBytes == 1)
            opcodeBytes = std::format("{:02X}", read(pc));
        else if (instr.nBytes == 2)
            opcodeBytes = std::format("{:02X} {:02X}", read(pc), read(pc+1));
        else if (instr.nBytes == 3)
            opcodeBytes = std::format("{:02X} {:02X} {:02X}", read(pc), read(pc+1), read(pc+2));

        std::string args;
        if (instr.addrmode == &Cpu::AddrABS)
           args = std::format("${:04X}", (read(pc+2)<<8) + read(pc+1));
        if (instr.addrmode == &Cpu::AddrABS)
           args = std::format("${:04X}", (read(pc+2)<<8) + read(pc+1));
        else if (instr.addrmode == &Cpu::AddrABX)
           args = std::format("${:04X},X", (read(pc+2)<<8) + read(pc+1));
        else if (instr.addrmode == &Cpu::AddrABY)
           args = std::format("${:04X},Y", (read(pc+2)<<8) + read(pc+1));
        else if (instr.addrmode == &Cpu::AddrZP0)
           args = std::format("${:02X}", read(pc+1));
        else if (instr.addrmode == &Cpu::AddrZPX)
           args = std::format("${:02X},X", read(pc+1));
        else if (instr.addrmode == &Cpu::AddrZPY)
           args = std::format("${:02X},Y", read(pc+1));
        else if (instr.addrmode == &Cpu::AddrIZX)
           args = std::format("(${:02X},X)", read(pc+1));
        else if (instr.addrmode == &Cpu::AddrIZY)
           args = std::format("(${:02X}),Y", read(pc+1));
        else if (instr.addrmode == &Cpu::AddrIMM)
           args = std::format("#${:02X}", read(pc+1));
        else if (instr.addrmode == &Cpu::AddrACC)
           args = std::format("A");
        else if (instr.addrmode == &Cpu::AddrREL || instr.addrmode == &Cpu::AddrIND)
           args = std::format("${:04X}", m_targetAddress);

        else
            args = "";


        std::print("{:04X}  {:9s} {} {:27s} ", pc, opcodeBytes, instr.name, args);
        std::print("A:{:02X} X:{:02X} Y:{:02X} P:{:02X} SP:{:02X} ", A, X, Y, P, SP);
        std::println("PPU:{:3d},{:3d} CYC:{:d}", m_bus->ppu()->scanline(), m_bus->ppu()->dot(), 
                    m_nTotCycles);
}

