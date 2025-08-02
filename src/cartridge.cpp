#include "cartridge.hpp"

#include <cassert>
#include <print>
#include <filesystem>
#include <stdexcept>
#include <cstring>

namespace fs = std::filesystem;

#define KBYTES_8   8192
#define KBYTES_16 16384



Cartridge::Cartridge(const char* filename) {
    if (!load(filename)) {
        throw std::runtime_error(std::format("Failed to load ROM [{}]", filename));
    }
}

uint8_t Cartridge::nProgBlocks() { return m_rawData[4]; }
uint8_t Cartridge::nCharBlocks() { return m_rawData[5]; }
std::bitset<8> Cartridge::flags6() { return std::bitset<8>(m_rawData[6]); }
std::bitset<8> Cartridge::flags7() { return std::bitset<8>(m_rawData[7]); }
bool Cartridge::hasTrainer() { return (flags6().test(2)); }


const uint8_t Cartridge::prgData(uint8_t iBlock, uint16_t addr)
{
    assert(addr < KBYTES_16);
    return *(m_progData + iBlock*KBYTES_16 + addr);
}

const uint8_t Cartridge::chrData(uint8_t iBlock, uint16_t addr)
{
    assert(addr < KBYTES_8);
    return *(m_charData + iBlock*KBYTES_8 + addr);
}



bool Cartridge::load(const char* filename) {
    FILE *f;
    if (!(f = fopen(filename, "rb")))
    {
        std::println("!! {} is not a readable file", filename);
        return false;
    }

    m_rawDataSize = fread(m_rawData, sizeof(uint8_t), MAX_SIZE, f);    
    fclose(f);

    if (std::strncmp((const char *)m_rawData, "NES", 3) || (m_rawData[3] != 0x1A)) {
        std::println("!! not a NES ROM");
        return false;
    }

    m_filename = fs::path(filename).filename();
    m_progData = m_rawData + 16 + (hasTrainer() ? 512 : 0);
    m_charData = m_progData + nProgBlocks() * KBYTES_16;

    return true;
}

void Cartridge::printDiagnostics()
{
    std::println("---- Cartridge diagnostics ----");
    std::println("file name: {}", m_filename);
    std::println("raw data size: {}", m_rawDataSize);
    std::println("nProgBlocks: {}", nProgBlocks());
    std::println("nCharBlocks: {}", nCharBlocks());
    std::println("flags6: {}", flags6().to_string());
    std::println("flags7: {}", flags7().to_string());
    std::println("hasTrainer: {}", hasTrainer());
    std::println("-------------------------------");

}

