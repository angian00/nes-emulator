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
        std::println("[{:04d}] PC: ${:04X} opcode: 0x{:02X} {}", 
            nProcessedInstr, PC-1, opcode, instr.name);

        m_currAddrMode = instr.addrmode;
        uint8_t extraCycles1 = (this->*instr.addrmode)();
        uint8_t extraCycles2 = (this->*instr.operate)();  
        //nWaitCycles += (extraCycles1 & extraCycles2);
    }

    nWaitCycles--;
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
    X = A;
    setFlag(FlagIndex::Zero,     X == 0x00);
    setFlag(FlagIndex::Negative, X & 0x80);
    return 0x00;
}
uint8_t Cpu::OpTAY()
{
    Y = A;
    setFlag(FlagIndex::Zero,     Y == 0x00);
    setFlag(FlagIndex::Negative, Y & 0x80);
    return 0x00;
}
uint8_t Cpu::OpTXA()
{
    A = X;
    setFlag(FlagIndex::Zero,     A == 0x00);
    setFlag(FlagIndex::Negative, A & 0x80);
    return 0x00;
}
uint8_t Cpu::OpTYA()
{
    A = Y;
    setFlag(FlagIndex::Zero,     A == 0x00);
    setFlag(FlagIndex::Negative, A & 0x80);
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
    pushStack(A);
    
    return 0x00;
}
uint8_t Cpu::OpPHP()
{
    pushStack(P);
    return 0x00;
}
uint8_t Cpu::OpPLA()
{
    A = popStack();
    setFlag(FlagIndex::Zero,     A == 0x00);
    setFlag(FlagIndex::Negative, A & 0x80);
    return 0x00;
}
uint8_t Cpu::OpPLP()
{
    P = popStack();
    return 0x00;
}

//-- Logical --
uint8_t Cpu::OpAND()
{
    A = A & read(targetAddress);
    setFlag(FlagIndex::Zero,     A == 0x00);
    setFlag(FlagIndex::Negative, A & 0x80);
    return 0x00;
}
uint8_t Cpu::OpEOR()
{
    A = A ^ read(targetAddress);
    setFlag(FlagIndex::Zero,     A == 0x00);
    setFlag(FlagIndex::Negative, A & 0x80);
    return 0x00;
}
uint8_t Cpu::OpORA()
{
    A = A | read(targetAddress);
    setFlag(FlagIndex::Zero,     A == 0x00);
    setFlag(FlagIndex::Negative, A & 0x80);
    return 0x00;
}
uint8_t Cpu::OpBIT()
{
    uint8_t mValue = read(targetAddress);
    setFlag(FlagIndex::Zero,     (A & mValue) == 0x00);
    setFlag(FlagIndex::Overflow, ((mValue >> 6) & 0x1) == 0x1);
    setFlag(FlagIndex::Negative, ((mValue >> 7) & 0x1) == 0x1);
    return 0x00;
}

//-- Arithmetic --
uint8_t Cpu::OpADC()
{
    uint16_t res = A + read(targetAddress) + (hasFlag(FlagIndex::Carry) ? 0x0100 : 0x0000);
    A = (res & 0xFF);
    setFlag(FlagIndex::Carry,    res >= 0x0100);
    setFlag(FlagIndex::Zero,     A == 0x00);
    setFlag(FlagIndex::Negative, A & 0x80);
    //see http://www.6502.org/tutorials/vflag.html for Overflow
    //setFlag(FlagIndex::Overflow, (((int8_t)A) >= -128) && (((int8_t)A) <= 127)); //TODO: Overflow
  
    return 0x00;
}
uint8_t Cpu::OpSBC()
{
    uint16_t res = A - read(targetAddress) + (hasFlag(FlagIndex::Carry) ? 0x0100 : 0x0000) - 1;
    A = (res & 0xFF);
    setFlag(FlagIndex::Carry,    !(res >= 0x0100));
    setFlag(FlagIndex::Zero,     A == 0x00);
    setFlag(FlagIndex::Negative, A & 0x80);
    //see http://www.6502.org/tutorials/vflag.html for Overflow
    //setFlag(FlagIndex::Overflow, (((int8_t)A) < -128) || (((int8_t)A) > 127)); //TODO: Overflow
  
    return 0x00;
}
uint8_t Cpu::OpCMP()
{
    uint8_t res = A - read(targetAddress);
    setFlag(FlagIndex::Carry,    res > 0x00);
    setFlag(FlagIndex::Zero,     res == 0x00);
    setFlag(FlagIndex::Negative, res & 0x80);
    return 0x00;
}
uint8_t Cpu::OpCPX()
{
    uint8_t res = X - read(targetAddress);
    setFlag(FlagIndex::Carry,    res > 0x00);
    setFlag(FlagIndex::Zero,     res == 0x00);
    setFlag(FlagIndex::Negative, res & 0x80);
    return 0x00;
}
uint8_t Cpu::OpCPY()
{
    uint8_t res = Y - read(targetAddress);
    setFlag(FlagIndex::Carry,    res > 0x00);
    setFlag(FlagIndex::Zero,     res == 0x00);
    setFlag(FlagIndex::Negative, res & 0x80);
    return 0x00;
}

//-- Increments & Decrements --
uint8_t Cpu::OpINC()
{
    uint8_t res = read(targetAddress) + 1;
    write(targetAddress, res);
    setFlag(FlagIndex::Zero,     res == 0x00);
    setFlag(FlagIndex::Negative, res & 0x80);
    return 0x00;
}
uint8_t Cpu::OpINX()
{
    X = X + 1;
    setFlag(FlagIndex::Zero,     X == 0x00);
    setFlag(FlagIndex::Negative, X & 0x80);
    return 0x00;
}
uint8_t Cpu::OpINY()
{
    Y = Y + 1;
    setFlag(FlagIndex::Zero,     Y == 0x00);
    setFlag(FlagIndex::Negative, Y & 0x80);
    return 0x00;
}
uint8_t Cpu::OpDEC()
{
    uint8_t res = read(targetAddress) - 1;
    write(targetAddress, res);
    setFlag(FlagIndex::Zero,     res == 0x00);
    setFlag(FlagIndex::Negative, res & 0x80);
    return 0x00;
}
uint8_t Cpu::OpDEX()
{
    X = X - 1;
    setFlag(FlagIndex::Zero,     X == 0x00);
    setFlag(FlagIndex::Negative, X & 0x80);
    return 0x00;
}
uint8_t Cpu::OpDEY()
{
    Y = Y - 1;
    setFlag(FlagIndex::Zero,     Y == 0x00);
    setFlag(FlagIndex::Negative, Y & 0x80);
    return 0x00;
}

//-- Shifts --
uint8_t Cpu::OpASL()
{
    if (m_currAddrMode == &Cpu::AddrACC) {
        setFlag(FlagIndex::Carry, A & 0x80);
        A <<= 1;
        setFlag(FlagIndex::Zero, A == 0x00);
        setFlag(FlagIndex::Negative, A & 0x80);        
    }
    else
    {
        uint8_t value = read(targetAddress);
        setFlag(FlagIndex::Carry, value & 0x80);
        value <<= 1;
        setFlag(FlagIndex::Zero, value == 0x00);
        setFlag(FlagIndex::Negative, value & 0x80);
        write(targetAddress, value);
    }

    return 0x00;
}
uint8_t Cpu::OpLSR()
{
    if (m_currAddrMode == &Cpu::AddrACC) {
        setFlag(FlagIndex::Carry, A & 0x01);
        A >>= 1;
        setFlag(FlagIndex::Zero, A == 0x00);
        //setFlag(FlagIndex::Negative, A & 0x80);        
    }
    else
    {
        uint8_t value = read(targetAddress);
        setFlag(FlagIndex::Carry, value & 0x01);
        value >>= 1;
        setFlag(FlagIndex::Zero, value == 0x00);
        //setFlag(FlagIndex::Negative, value & 0x80);
        write(targetAddress, value);
    }

    return 0x00;
}
uint8_t Cpu::OpROL()
{
    uint8_t oldCarry = hasFlag(FlagIndex::Carry);

    if (m_currAddrMode == &Cpu::AddrACC) {
        setFlag(FlagIndex::Carry, A & 0x80);
        A <<= 1;
        if (oldCarry)
            A |= 0x01;
        else
            A &= (~0x01);
        setFlag(FlagIndex::Zero, A == 0x00);
        setFlag(FlagIndex::Negative, A & 0x80);
    }
    else
    {
        uint8_t value = read(targetAddress);
        setFlag(FlagIndex::Carry, value & 0x80);
        value <<= 1;
        if (oldCarry)
            value |= 0x01;
        else
            value &= (~0x01);
        setFlag(FlagIndex::Zero, value == 0x00);
        setFlag(FlagIndex::Negative, value & 0x80);
        write(targetAddress, value);
    }

    return 0x00;
}
uint8_t Cpu::OpROR()
{
    uint8_t oldCarry = hasFlag(FlagIndex::Carry);

    if (m_currAddrMode == &Cpu::AddrACC) {
        setFlag(FlagIndex::Carry, A & 0x01);
        A >>= 1;
        if (oldCarry)
            A |= 0x80;
        else
            A &= (~0x80);
        setFlag(FlagIndex::Zero, A == 0x00);
        setFlag(FlagIndex::Negative, A & 0x80);        
    }
    else
    {
        uint8_t value = read(targetAddress);
        setFlag(FlagIndex::Carry, value & 0x01);
        value >>= 1;
        if (oldCarry)
            value |= 0x80;
        else
            value &= (~0x80);
        setFlag(FlagIndex::Zero, value == 0x00);
        setFlag(FlagIndex::Negative, value & 0x80);
        write(targetAddress, value);
    }
    return 0x00;
}

//-- Jumps & Calls --
uint8_t Cpu::OpJMP()
{
    PC = targetAddress;
    return 0x00;
}
uint8_t Cpu::OpJSR()
{
    pushStack(PC); //CHECK: PC - 1?
    PC = targetAddress;
    return 0x00;
}
uint8_t Cpu::OpRTS()
{
    PC = popStack();
    return 0x00;
}

//-- Branches --
uint8_t Cpu::OpBCC()
{
    if (!hasFlag(FlagIndex::Carry))
        PC = targetAddress;

    return 0x00;
}
uint8_t Cpu::OpBCS()
{
    if (hasFlag(FlagIndex::Carry))
        PC = targetAddress;

    return 0x00;
}
uint8_t Cpu::OpBEQ()
{
    if (hasFlag(FlagIndex::Zero))
        PC = targetAddress;

    return 0x00;
}
uint8_t Cpu::OpBMI()
{
    if (hasFlag(FlagIndex::Negative))
        PC = targetAddress;

    return 0x00;
}
uint8_t Cpu::OpBNE()
{
    if (!hasFlag(FlagIndex::Zero))
        PC = targetAddress;

    return 0x00;
}
uint8_t Cpu::OpBPL()
{
    if (!hasFlag(FlagIndex::Negative))
        PC = targetAddress;

    return 0x00;
}
uint8_t Cpu::OpBVC()
{
    if (!hasFlag(FlagIndex::Overflow))
        PC = targetAddress;

    return 0x00;
}
uint8_t Cpu::OpBVS()
{
    if (hasFlag(FlagIndex::Overflow))
        PC = targetAddress;

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
    pushStack((PC & 0xff00) >> 8);
    pushStack(PC & 0xff);
    //CHECK: " The return address pushed to the stack is PC+2, 
    // providing an extra byte of spacing for a break mark
    // (identifying a reason for the break.) "
    pushStack(P);

    uint16_t lo = m_bus->read(0xFFFC);
    uint16_t hi = m_bus->read(0xFFFD);
    PC = (hi << 8) | lo;

    setFlag(FlagIndex::InterruptDisable, true);
    return 0x00;
}
uint8_t Cpu::OpNOP()
{
    //do nothing
    return 0x00;
}
uint8_t Cpu::OpRTI()
{
    P = popStack();
    uint16_t lo = popStack();
    uint16_t hi = popStack();
    PC = (hi << 8) | lo;

    return 0x00;
}

