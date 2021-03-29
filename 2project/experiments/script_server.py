import os
import sys
from multiprocessing import Process, Pool
import threading
traces = os.listdir("/home/cx872/Documents/ECE_4100/2project/traces")

from math import log2

perceptron_pg_ranges = []


for p in range(1, 64):
    for g in range(1, 64):
        theta = 1.93 * g + 14
        size = (int(log2(theta)) + 1) * (g+1) * (2**p)
        if size >= 8192 and size <= 65536:
            perceptron_pg_ranges.append((p, g))


print(perceptron_pg_ranges, len(perceptron_pg_ranges))


def run(rg):
    for trace in traces:
        name = "traces/" + trace
        for (p, g) in perceptron_pg_ranges[rg[0]:rg[1]]:
            os.system("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 3, g, p))
            print("./cmake-build-debug/branchsim -i %s -o %d -g %d -p %d" % (name, 3, g, p))
            

range_pool = []

for i in range(7):
    range_pool.append((i*28, i*28+28))
print(range_pool)


if __name__ == '__main__':
    with Pool(7) as p:
        p.map(run, range_pool)

    