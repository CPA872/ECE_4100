d = {};
d[1] = "NOP"
d[2] = "ADD"
d[3] = "MUL"
d[4] = "LD "
d[5] = "ST "
d[6] = "BR "
d[7] = "INT"


translated = open("trans_bwaves.txt", "w")
trace = open("bwaves603_2M.trace", "r")

for line in trace.readlines():
    (addr, opcode, dest_reg, src1, src2, mem_addr, br_target, br_taken) = \
            line.split(" ")[:-1]
    translated.write(d[int(opcode)] + " R" + src1 + "\t R" + src2 + "\n")
    
translated.close()
trace.close()

