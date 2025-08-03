#include "bit_operations.hpp"

#include <print>
#include <bitset>


void testAssignBits(uint16_t dest, uint16_t src, uint8_t destStart, uint8_t srcStart, uint8_t len, uint16_t expected);


int main(int argc, char* argv[])
{
    uint16_t dest;
    uint16_t src;

    std::println("Testing bit operations");

    testAssignBits(0x00ff, 0x0000, 0, 0, 8, 0x0000);
    testAssignBits(0x0000, 0x00ff, 0, 0, 8, 0x00ff);
    testAssignBits(0x0000, 0xff00, 0, 8, 8, 0x00ff);
    testAssignBits(0xffff, 0xcc00, 4, 8, 8, 0xfccf);

    testAssignBits(0xffff, 0x00, 10, 0, 2, 0b1111001111111111);
    testAssignBits(0x0000, 0xff, 10, 0, 2, ~(0b1111001111111111));

    std::println("All tests ok");
}


void testAssignBits(uint16_t dest, uint16_t src, uint8_t destStart, uint8_t srcStart, uint8_t len, uint16_t expected)
{
    uint16_t res = dest;
    assignBits(&res, src, destStart, srcStart, len);
    
    std::print("[{}] [{}-{}] <-- [{}] [{}-{}] == [{}]    ", 
        std::bitset<16>(dest).to_string(), destStart, destStart+len, 
        std::bitset<16>(src).to_string(),   srcStart,  srcStart+len, 
        std::bitset<16>(res).to_string());
        
    if (res == expected)
        std::print("OK");
    else
        std::print("!! KO !!  , expected: [{}", std::bitset<16>(expected).to_string());
    
    std::println();
}