from multiprocessing import Pool
import os
p = 64
r = 12
i = 10000

traces = ['traces/bwaves603_2M.trace', 'traces/gcc602_2M.trace', 'traces/mcf605_2M.trace', 'traces/perlbench600_2M.trace']

#jklf

rr = [r for r in range(12, 48)]

total = len(rr) * 4

def run(index):
    curr = 0
    for r in rr:
        for i_cycles in [1000, 10000, 100000]:
            cmd = "./procsim -r %d -i %d -t %s" % (r, i_cycles, traces[index])
            os.system(cmd)
            curr += 1
            print("=========  %d out of %d  ========" % (curr, total))


if __name__ == "__main__":
    with Pool(4) as p:
        p.map(run, [0, 1, 2, 3])
