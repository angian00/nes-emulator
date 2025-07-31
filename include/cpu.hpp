#pragma once

#include <cstdint>
#include <bitset>

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


class Cpu
{
    friend struct Instruction; 

    static const uint16_t STACK_START = 0x100;

public:
    Cpu() { initInstrLookupTable(); };
    
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
    uint8_t OpADC(); //TODO
    uint8_t OpAND(); //TODO
    uint8_t OpASL(); //TODO
    uint8_t OpBCC();
    uint8_t OpBCS();
    uint8_t OpBEQ();
    uint8_t OpBIT(); //TODO
    uint8_t OpBMI();
    uint8_t OpBNE();
    uint8_t OpBPL();
    uint8_t OpBRK(); //TODO
    uint8_t OpBVC();
    uint8_t OpBVS();
    uint8_t OpCLC();
    uint8_t OpCLD();
    uint8_t OpCLI();
    uint8_t OpCLV();
    uint8_t OpCMP(); //TODO
    uint8_t OpCPX(); //TODO
    uint8_t OpCPY(); //TODO
    uint8_t OpDEC(); //TODO
    uint8_t OpDEX(); //TODO
    uint8_t OpDEY(); //TODO
    uint8_t OpEOR(); //TODO
    uint8_t OpINC(); //TODO
    uint8_t OpINX(); //TODO
    uint8_t OpINY(); //TODO
    uint8_t OpJMP(); //TODO
    uint8_t OpJSR(); //TODO
    uint8_t OpLDA();
    uint8_t OpLDX();
    uint8_t OpLDY();
    uint8_t OpLSR(); //TODO
    uint8_t OpNOP(); //TODO
    uint8_t OpORA(); //TODO
    uint8_t OpPHA(); //TODO
    uint8_t OpPHP(); //TODO
    uint8_t OpPLA(); //TODO
    uint8_t OpPLP(); //TODO
    uint8_t OpROL(); //TODO
    uint8_t OpROR(); //TODO
    uint8_t OpRTI(); //TODO
    uint8_t OpRTS(); //TODO
    uint8_t OpSBC(); //TODO
    uint8_t OpSEC();
    uint8_t OpSED();
    uint8_t OpSEI();
    uint8_t OpSTA();
    uint8_t OpSTX();
    uint8_t OpSTY();
    uint8_t OpTAX(); //TODO
    uint8_t OpTAY(); //TODO
    uint8_t OpTSX(); //TODO
    uint8_t OpTXA(); //TODO
    uint8_t OpTXS(); //TODO
    uint8_t OpTYA(); //TODO


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
    uint8_t fetched;
    uint8_t opcode; //TODO
    uint8_t waitCycles;
    uint16_t targetAddress;

    bool hasFlag(FlagIndex flagIndex);
    void setFlag(FlagIndex flagIndex, bool value);

    
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);
    uint8_t readStack(uint8_t spRel);
    void writeStack(uint8_t spRel, uint8_t value);

    void initInstrLookupTable();

};
