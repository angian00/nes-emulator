#pragma once

#include <cstdint>
#include <string>
#include <bitset>

class Cartridge
{
public:
    static const uint32_t MAX_SIZE = 1024*1024;

    Cartridge(const char* romPath);

    std::string const filename() { return m_filename; };
    uint8_t nProgBlocks();
    uint8_t nCharBlocks();
    uint8_t prgData(uint8_t iBlock, uint16_t addr);

    void printDiagnostics();
    
private:
    std::string m_filename;

    uint8_t m_rawData[MAX_SIZE] {};
    uint32_t m_rawDataSize;
    uint8_t* m_progData;
    uint8_t* m_charData;

    bool load(const char* romPath);
    std::bitset<8> flags6();
    std::bitset<8> flags7();
    bool hasTrainer();
};
