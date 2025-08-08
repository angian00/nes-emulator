
#include "bit_operations.hpp"

#include <cassert>


void assignBits(uint16_t* dest, uint16_t src, uint8_t destPosStart, uint8_t srcPosStart, uint8_t len)
{
    assert(destPosStart < 16);
    assert(srcPosStart < 16);
    assert(len > 0);

    uint16_t maskedSrcValue = (src >> srcPosStart) & ((1 << len) - 1);
    *dest = (*dest) & ~( ((1 << len) - 1) << destPosStart);
    *dest = (*dest) | (maskedSrcValue << destPosStart);
}

void assignBits(uint8_t* dest, uint8_t src, uint8_t destPosStart, uint8_t srcPosStart, uint8_t len)
{
    assert(destPosStart < 8);
    assert(srcPosStart < 8);
    assert(len > 0);

    uint16_t maskedSrcValue = (src >> srcPosStart) & ((1 << len) - 1);
    *dest = (*dest) & ~( ((1 << len) - 1) << destPosStart);
    *dest = (*dest) | (maskedSrcValue << destPosStart);
}
