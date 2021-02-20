program = [

    0x0ECE4100,
    0x0ECE6100,
    0x00C1CD89,
    0x0054B0FA,
    0x00ABCD89,
    0x00CDEF9A,
    0x000CB017,
    0x1002ED12,
    0x1054B09B,
    0x00ABCD88,
    0x00EF7300,
    0x000CB680
]

def bin_gen(h, l):
    string = '0' * (31 - h) + '1' * (h - l + 1) + '0' * l
    # print(string)
    return string


print("\n-----------\n---Config A\n-------")
for inst in program:
    # config A
    tag    = (inst & int(bin_gen(31, 9), 2)) >> 9 
    index  = (inst & int(bin_gen(8, 4), 2)) >> 4
    offset = (inst & int(bin_gen(3, 0), 2))
    print("%10s   %12s, %8s, %8s" % (hex(inst), tag, index, offset))

print("\n-----------\n---Config B\n-------")
for inst in program:
    tag    = (inst & int(bin_gen(31, 5), 2)) >> 5
    index  = (inst & int(bin_gen(4, 4), 2)) >> 4
    offset = (inst & int(bin_gen(3, 0), 2))
    print("%10s   %12s, %8s, %8s" % (hex(inst), tag, index, offset))

print("\n-----------\n---Config C\n-------")
for inst in program:
    tag    = (inst & int(bin_gen(31, 7), 2)) >> 7
    index  = (inst & int(bin_gen(6, 4), 2)) >> 4
    offset = (inst & int(bin_gen(3, 0), 2))
    print("%10s   %12s, %8s, %8s" % (hex(inst), tag, index, offset))


print("\nquestion 2\n")
for inst in program:
    print("%12s  %32s" % (str(hex(inst))[:-1], str(bin(inst))[:-4]))
