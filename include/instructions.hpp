#pragma once


#include <cstdint>
#include <string>

class Cpu;

struct Instruction
{
    std::string name;
    uint8_t (Cpu::*operate)(void) = nullptr;
    uint8_t (Cpu::*addrmode)(void) = nullptr;
    uint8_t nBytes = 0;
    uint8_t nCycles = 0;
};

extern struct Instruction instructionLookupTable[256];

void initInstrLookupTable();
