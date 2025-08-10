#pragma once

#include <cstdint>

#include "instructions.hpp"

class Bus;


enum FlagIndex {
// NV1B DIZC
// |||| ||||
// |||| |||+- Carry
// |||| ||+-- Zero
// |||| |+--- Interrupt Disable
// |||| +---- Decimal
// |||+------ (No CPU effect; see: the B flag)
// ||+------- (No CPU effect; always pushed as 1)
// |+-------- Overflow
// +--------- Negative
    Carry,
    Zero,
    InterruptDisable,
    Decimal,
    Unused1,
    Unused2,
    Overflow,
    Negative
};


bool isPageBreak(uint16_t addr1, uint16_t addr2);

class Cpu
{
    friend struct Instruction; 

    static const uint16_t STACK_START = 0x100;

public:
    Cpu() { initInstrLookupTable(); };
    void setTracing(bool value) { m_tracing = value; }
    void setPC(uint16_t value) { PC = value; }

    void connect(Bus* bus) { m_bus = bus; }
    void reset();
    void clock();
    
    //addressing modes
    uint8_t AddrABS();
    uint8_t AddrABX();
    uint8_t AddrABY();
    uint8_t AddrACC();
    uint8_t AddrIMM();
    uint8_t AddrIMP();
    uint8_t AddrIND();
    uint8_t AddrIZX();
    uint8_t AddrIZY();
    uint8_t AddrREL();
    uint8_t AddrZP0();
    uint8_t AddrZPX();
    uint8_t AddrZPY();

    //opcodes
    uint8_t OpADC();
    uint8_t OpAND();
    uint8_t OpASL();
    uint8_t OpBCC();
    uint8_t OpBCS();
    uint8_t OpBEQ();
    uint8_t OpBIT();
    uint8_t OpBMI();
    uint8_t OpBNE();
    uint8_t OpBPL();
    uint8_t OpBRK();
    uint8_t OpBVC();
    uint8_t OpBVS();
    uint8_t OpCLC();
    uint8_t OpCLD();
    uint8_t OpCLI();
    uint8_t OpCLV();
    uint8_t OpCMP();
    uint8_t OpCPX();
    uint8_t OpCPY();
    uint8_t OpDEC();
    uint8_t OpDEX();
    uint8_t OpDEY();
    uint8_t OpEOR();
    uint8_t OpINC();
    uint8_t OpINX();
    uint8_t OpINY();
    uint8_t OpJMP();
    uint8_t OpJSR();
    uint8_t OpLDA();
    uint8_t OpLDX();
    uint8_t OpLDY();
    uint8_t OpLSR();
    uint8_t OpNOP();
    uint8_t OpORA();
    uint8_t OpPHA();
    uint8_t OpPHP();
    uint8_t OpPLA();
    uint8_t OpPLP();
    uint8_t OpROL();
    uint8_t OpROR();
    uint8_t OpRTI();
    uint8_t OpRTS();
    uint8_t OpSBC();
    uint8_t OpSEC();
    uint8_t OpSED();
    uint8_t OpSEI();
    uint8_t OpSTA();
    uint8_t OpSTX();
    uint8_t OpSTY();
    uint8_t OpTAX();
    uint8_t OpTAY();
    uint8_t OpTSX();
    uint8_t OpTXA();
    uint8_t OpTXS();
    uint8_t OpTYA();

    uint8_t OpLAX();
    uint8_t OpSAX();
    uint8_t OpDCP();
    uint8_t OpISB();
    uint8_t OpSLO();
    uint8_t OpRLA();
    uint8_t OpSRE();
    uint8_t OpRRA();


private:
    Bus* m_bus;
    
    // registers
    uint8_t   A;  // Accumulator
    uint8_t   X;  // Index Register X
    uint8_t   Y;  // Index Register Y
    uint8_t  SP;  // Stack Pointer: 8-bit offset to be added to $0100
    uint16_t PC;  // Program Counter
    uint8_t   P;  // Processor status
    
    // other state
    uint8_t m_nWaitCycles;
    uint16_t m_targetAddress;
    uint32_t m_nProcessedInstr;
    uint32_t m_nTotCycles;
    uint8_t (Cpu::*m_currAddrMode)(void);

    bool m_tracing = false;

    //DEBUG PPU
    uint8_t m_scrollX;
    uint8_t m_scrollY;

    bool hasFlag(FlagIndex flagIndex);
    void setFlag(FlagIndex flagIndex, bool value);

    
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);
    void pushStack(uint8_t value);
    uint8_t popStack();
};
