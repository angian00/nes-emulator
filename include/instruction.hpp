#pragma once


struct Instruction
{
    std::string name;
    uint8_t (Cpu::*operate)(void) = nullptr;
    uint8_t (Cpu::*addrmode)(void) = nullptr;
    uint8_t nCycles = 0;
};

struct Instruction instructionLookupTable[256];
