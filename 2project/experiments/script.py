import os
import sys
from multiprocessing import Process, Pool
import threading
traces = os.listdir("/home/cx872/Documents/ECE_4100/2project/traces")
gshare_g_range = [12, 13, 14]

from math import log2
yeh_pg_ranges = []
perceptron_pg_ranges = []
for p in range(0, 100):
    for g in range(0, 100):
        size = p * (2**g) + 2** (p+1) 
        if size >= 8192 and size <= 65536:
            yeh_pg_ranges.append((p, g))



for p in range(1, 64):
    for g in range(1, 100):
        theta = 1.93 * g + 14
        size = (int(log2(theta)) + 1) * (g+1) * (2**p)
        if size >= 8192 and size <= 65536:
            perceptron_pg_ranges.append((p, g))


print(perceptron_pg_ranges, len(perceptron_pg_ranges))



# def r1():
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges[0:10]:
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 2, g, p))
#             print("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 2, g, p))

# def r2():
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges[11:20]:
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 2, g, p))
# def r3():
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges[21:30]:
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 2, g, p))
# def r4():
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges[31:40]:
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 2, g, p))
# def r5():
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges[41:50]:
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 2, g, p))

# def r6():
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges[51:60]:
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 2, g, p))
# def r7():
#     i = 0
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges[61:]:
#             i += 1
#             print("########\n########    %d out of %d\n########" % (i, len(traces) * 11))
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 2, g, p))

# def r1():
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges[0:10]:
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 2, g, p))
#             print("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 2, g, p))

# def r2():
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges[11:20]:
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 2, g, p))
# def r3():
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges[21:30]:
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 2, g, p))
# def r4():
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges[31:40]:
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 1, g, p))
# def r5():
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges[41:50]:
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 1, g, p))

# def r6():
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges[51:60]:
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 1, g, p))
# def r7():
#     i = 0
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges[61:]:
#             i += 1
#             print("########\n########    %d out of %d\n########" % (i, len(traces) * 11))
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 1, g, p))
# def r8():
#     for trace in traces:
#         name = "traces/" + trace
#         for (g, p) in yeh_pg_ranges:
#             os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 1, g, p))













def r1():
    for trace in traces:
        name = "traces/" + trace
        os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d" % (name, 1, 12))
        print("./cmake-build-debug/branchsim -i %s -o %d -g %d" % (name, 1, 12))

def r2():
    for trace in traces:
        name = "traces/" + trace
        os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d" % (name, 1, 13))
def r3():
    for trace in traces:
        name = "traces/" + trace
        os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d" % (name, 1, 14))
# def r4():
#     for trace in traces:
#         name = "traces/" + trace
#         os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d" % (name, 1, 7))
# def r5():
#     for trace in traces:
#         name = "traces/" + trace
#         os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d" % (name, 1, 8))

# def r6():
#     for trace in traces:
#         name = "traces/" + trace
#         os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d" % (name, 1, 9))
# def r7():
#     for trace in traces:
#         name = "traces/" + trace
#         os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d" % (name, 1, 10))
# def r8():
#     for trace in traces:
#         name = "traces/" + trace
#         os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d" % (name, 1, 11))


if __name__ == "__main__":
    t1 = threading.Thread(target=r1)
    t2 = threading.Thread(target=r2)
    t3 = threading.Thread(target=r3)
    # t4 = threading.Thread(target=r4)
    # t5 = threading.Thread(target=r5)
    # t6 = threading.Thread(target=r6)
    # t7 = threading.Thread(target=r7)
    # t8 = threading.Thread(target=r8)

    
    t1.start() 
    t2.start()
    t3.start()
    # t4.start()
    # t5.start()
    # t6.start()
    # t7.start()
    # t8.start()

    
    t1.join() 
    t2.join()
    t3.join()
    # t4.join()
    # t5.join()
    # t6.join()
    # t7.join()
    # t8.join()





