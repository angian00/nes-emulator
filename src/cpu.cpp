#include "cpu.hpp"

#include "bus.hpp"
#include "instruction.hpp"

#include <print>


void Cpu::reset()
{
    A = 0x00;
    X = 0x00;
    Y = 0x00;
    SP = 0xFD;     // offset to the start of the stack
    PC = 0x0000;
    P = 0x24; // IRQ disabled

    fetched = 0x00;
    opcode = 0x00;
    
    uint16_t lo = m_bus->read(0xFFFC);
    uint16_t hi = m_bus->read(0xFFFD);
    PC = (hi << 8) | lo;
    
    waitCycles = 8;

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

uint8_t Cpu::readStack(uint8_t spRel)
{
    return m_bus->read(STACK_START + spRel);
}
void Cpu::writeStack(uint8_t spRel, uint8_t value)
{
    m_bus->write(STACK_START + spRel, value);
}


void Cpu::clock()
{
    //std::println("Cpu::clock()");

    if (waitCycles == 0) {
        opcode = read(PC++);

        Instruction& instr = instructionLookupTable[opcode];
        waitCycles = instr.nCycles;
        std::println("PC: ${:04X} opcode: 0x{:02X} {}", PC-1, opcode, instr.name);

        uint8_t extraCycles1 = (this->*instr.addrmode)();
        uint8_t extraCycles2 = (this->*instr.operate)();  
        //waitCycles += (extraCycles1 & extraCycles2);
    }

    waitCycles--;
}


void Cpu::initInstrLookupTable()
{
    // instructionLookupTable[0x00] = { "XXX", &Cpu::Op, &Cpu::Addr, x };
    instructionLookupTable[0x10] = { "BPL", &Cpu::OpBPL, &Cpu::AddrREL, 2 };
    instructionLookupTable[0x78] = { "SEI", &Cpu::OpSEI, &Cpu::AddrIMP, 2 };
    instructionLookupTable[0x8D] = { "STA", &Cpu::OpSTA, &Cpu::AddrABS, 4 };
    instructionLookupTable[0x9A] = { "TXS", &Cpu::OpTXS, &Cpu::AddrIMP, 2 };
    instructionLookupTable[0xA2] = { "LDX", &Cpu::OpLDX, &Cpu::AddrIMM, 2 };
    instructionLookupTable[0xA9] = { "LDA", &Cpu::OpLDA, &Cpu::AddrIMM, 2 };
    instructionLookupTable[0xAD] = { "LDA", &Cpu::OpLDA, &Cpu::AddrABS, 4 };
    instructionLookupTable[0xD8] = { "CLD", &Cpu::OpCLD, &Cpu::AddrIMP, 2 };

    // instructionLookupTable[0xA5] = { "LDA", &Cpu::OpLDA, &Cpu::AddrZP0, 3 };
}


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
    uint16_t rel = read(PC);
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


//---- opcode implementation ----
// see https://www.nesdev.org/obelisk-6502-guide/instructions.html

//-- Load/Store --
uint8_t Cpu::OpLDA()
{
    A = read(targetAddress);
    setFlag(FlagIndex::Zero,     A == 0x00);
    setFlag(FlagIndex::Negative, A & 0x80);
    return 0x00;
}
uint8_t Cpu::OpLDX()
{
    X = read(targetAddress);
    setFlag(FlagIndex::Zero,     X == 0x00);
    setFlag(FlagIndex::Negative, X & 0x80);
    return 0x00;
}
uint8_t Cpu::OpLDY()
{
    Y = read(targetAddress);
    setFlag(FlagIndex::Zero,     Y == 0x00);
    setFlag(FlagIndex::Negative, Y & 0x80);
    return 0x00;
}
uint8_t Cpu::OpSTA()
{
    write(targetAddress, A);
    return 0x00;
}
uint8_t Cpu::OpSTX()
{
    write(targetAddress, X);
    return 0x00;
}
uint8_t Cpu::OpSTY()
{
    write(targetAddress, Y);
    return 0x00;
}

//-- Register Transfers --
uint8_t Cpu::OpTAX()
{
    return 0x00;
}
uint8_t Cpu::OpTAY()
{
    return 0x00;
}
uint8_t Cpu::OpTXA()
{
    return 0x00;
}
uint8_t Cpu::OpTYA()
{
    return 0x00;
}

//-- Stack --
uint8_t Cpu::OpTSX()
{
    X = SP;
    setFlag(FlagIndex::Zero,     X == 0x00);
    setFlag(FlagIndex::Negative, X & 0x80);
    return 0x00;
}
uint8_t Cpu::OpTXS()
{
    X = SP;
    return 0x00;
}
uint8_t Cpu::OpPHA()
{
    if (SP == 0xFF)
        throw std::runtime_error("stack overflow");

    writeStack(SP++, A);

    return 0x00;
}
uint8_t Cpu::OpPHP()
{
    if (SP == 0xFF)
        throw std::runtime_error("stack overflow");

    writeStack(SP++, P);

    return 0x00;
}
uint8_t Cpu::OpPLA()
{
    if (SP == 0x00)
        throw std::runtime_error("stack underflow");

    setFlag(FlagIndex::Zero,     A == 0x00);
    setFlag(FlagIndex::Negative, A & 0x80);
    
    A = readStack(--SP);

    return 0x00;
}
uint8_t Cpu::OpPLP()
{
    if (SP == 0x00)
        throw std::runtime_error("stack underflow");

    P = readStack(--SP);
    return 0x00;
}

//-- Logical --
uint8_t Cpu::OpAND()
{
    return 0x00;
}
uint8_t Cpu::OpEOR()
{
    return 0x00;
}
uint8_t Cpu::OpORA()
{
    return 0x00;
}
uint8_t Cpu::OpBIT()
{
    return 0x00;
}

//-- Arithmetic --
uint8_t Cpu::OpADC()
{
    return 0x00;
}
uint8_t Cpu::OpSBC()
{
    return 0x00;
}
uint8_t Cpu::OpCMP()
{
    return 0x00;
}
uint8_t Cpu::OpCPX()
{
    return 0x00;
}
uint8_t Cpu::OpCPY()
{
    return 0x00;
}

//-- Increments & Decrements --
uint8_t Cpu::OpINC()
{
    return 0x00;
}
uint8_t Cpu::OpINX()
{
    return 0x00;
}
uint8_t Cpu::OpINY()
{
    return 0x00;
}
uint8_t Cpu::OpDEC()
{
    return 0x00;
}
uint8_t Cpu::OpDEX()
{
    return 0x00;
}
uint8_t Cpu::OpDEY()
{
    return 0x00;
}

//-- Shifts --
uint8_t Cpu::OpASL()
{
    return 0x00;
}
uint8_t Cpu::OpLSR()
{
    return 0x00;
}
uint8_t Cpu::OpROL()
{
    return 0x00;
}
uint8_t Cpu::OpROR()
{
    return 0x00;
}

//-- Jumps & Calls --
uint8_t Cpu::OpJMP()
{
    return 0x00;
}
uint8_t Cpu::OpJSR()
{
    return 0x00;
}
uint8_t Cpu::OpRTS()
{
    return 0x00;
}

//-- Branches --
uint8_t Cpu::OpBCC()
{
    if (!hasFlag(FlagIndex::Carry))
        SP = targetAddress;

    return 0x00;
}
uint8_t Cpu::OpBCS()
{
    if (hasFlag(FlagIndex::Carry))
        SP = targetAddress;

    return 0x00;
}
uint8_t Cpu::OpBEQ()
{
    if (hasFlag(FlagIndex::Zero))
        SP = targetAddress;

    return 0x00;
}
uint8_t Cpu::OpBMI()
{
    if (hasFlag(FlagIndex::Negative))
        SP = targetAddress;

    return 0x00;
}
uint8_t Cpu::OpBNE()
{
    if (!hasFlag(FlagIndex::Zero))
        SP = targetAddress;

    return 0x00;
}
uint8_t Cpu::OpBPL()
{
    if (!hasFlag(FlagIndex::Negative))
        SP = targetAddress;

    return 0x00;
}
uint8_t Cpu::OpBVC()
{
    if (!hasFlag(FlagIndex::Overflow))
        SP = targetAddress;

    return 0x00;
}
uint8_t Cpu::OpBVS()
{
    if (hasFlag(FlagIndex::Overflow))
        SP = targetAddress;

    return 0x00;
}

//-- Status Flag Changes --
uint8_t Cpu::OpCLC()
{
    setFlag(FlagIndex::Carry, false);
    return 0x00;
}
uint8_t Cpu::OpCLD()
{
    setFlag(FlagIndex::Decimal, false);
    return 0x00;
}
uint8_t Cpu::OpCLI()
{
    setFlag(FlagIndex::InterruptDisable, false);
    return 0x00;
}
uint8_t Cpu::OpCLV()
{
    setFlag(FlagIndex::Overflow, false);
    return 0x00;
}
uint8_t Cpu::OpSEC()
{
    setFlag(FlagIndex::Carry, true);
    return 0x00;
}
uint8_t Cpu::OpSED()
{
    setFlag(FlagIndex::Decimal, true);
    return 0x00;
}
uint8_t Cpu::OpSEI()
{
    setFlag(FlagIndex::InterruptDisable, true);
    return 0x00;
}

//-- System Functions --
uint8_t Cpu::OpBRK()
{
    return 0x00;
}
uint8_t Cpu::OpNOP()
{
    return 0x00;
}
uint8_t Cpu::OpRTI()
{
    return 0x00;
}

