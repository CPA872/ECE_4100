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