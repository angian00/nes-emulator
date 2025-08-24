#!/bin/env python3

def main():
    with open("instruction_list.txt", "r") as f:
        file_lines = f.readlines()
    
    for line in file_lines:
        line = line.strip()
        if line == "" or line.startswith("MODE") or line.startswith("#"):
            continue

        mode_str = line[:14].strip()
        mnemonic  = line[14:17]
        hex = line[28:31]
        n_bytes = line[33:34]
        n_cycles = line[37:38]
        #print(mode, hex, n_cycles)
        #print(mode_str, mnemonic)

        if mode_str == "Absolute":
            addr_mode = "ABS"
        elif mode_str == "Absolute,X":
            addr_mode = "ABX"
        elif mode_str == "Absolute,Y":
            addr_mode = "ABY"
        elif mode_str == "Accumulator":
            addr_mode = "ACC"
        elif mode_str == "Immediate":
            addr_mode = "IMM"
        elif mode_str == "Implied":
            addr_mode = "IMP"
        elif mode_str == "Indirect":
            addr_mode = "IND"
        elif mode_str == "Indirect,X":
            addr_mode = "IZX"
        elif mode_str == "Indirect,Y":
            addr_mode = "IZY"
        elif mode_str == "Relative":
            addr_mode = "REL"
        elif mode_str == "Zero Page":
            addr_mode = "ZP0"
        elif mode_str == "Zero Page,X":
            addr_mode = "ZPX"
        elif mode_str == "Zero Page,Y":
            addr_mode = "ZPY"
        else:
            raise ValueError(f"Unmapped mode_str: {mode_str}")
        
        #"instructionLookupTable[0x10] = { "BPL", &Cpu::OpBPL, &Cpu::AddrREL, 2 };"
        #instructionLookupTable[0x10] = { "BPL", &Cpu::OpBPL, &Cpu::AddrREL, 2 };
        print(f"instructionLookupTable[0x{hex[1:]}] = {{ \"{mnemonic}\", &Cpu::Op{mnemonic}, &Cpu::Addr{addr_mode}, {n_bytes}, {n_cycles} }};")



if __name__ == "__main__":
    main()
