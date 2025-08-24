#!/bin/env python3

filename_log = "output.log"
filename_ref = "nestest/nestest.log"

def main():
    with open(filename_log, "r") as f:
        log_lines = f.readlines()
    
    with open(filename_ref, "r") as f:
        ref_lines = f.readlines()
    
    i_log = 0
    while i_log < len(log_lines) and len(log_lines[i_log]) <= 32:
        i_log = i_log + 1

    i_ref = 0

    n_lines = min(len(log_lines) - i_log, len(ref_lines))
    print()

    while i_ref < n_lines:
        ref_line = ref_lines[i_ref]
        pc_ref = ref_line[0:4]
        opcode_ref = ref_line[16:19]
        registers_ref = ref_line[48:74]
        cycles_ref = ref_line[86:]
        
        #print(pc_ref, opcode_ref, registers_ref)

        log_line = log_lines[i_log]
        pc_log = log_line[0:4]
        opcode_log = log_line[16:19]
        registers_log = log_line[48:74]
        cycles_log = log_line[86:]

        #print(pc_log, opcode_log, registers_log)

        print(f"{ref_line.strip()}")

        if pc_log != pc_ref or opcode_log != opcode_ref or registers_log != registers_ref or cycles_ref != cycles_log:
            print(f"!! Lines [{i_ref+1}] do not match:")
            print(f"log: {log_line.strip()}")
            print(f"ref: {ref_line.strip()}")
            break

        i_ref = i_ref + 1
        i_log = i_log + 1


if __name__ == "__main__":
    main()
