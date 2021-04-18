from multiprocessing import Pool
import os
p = 64

r = 12
i = 10000

traces = ['traces/bwaves603_2M.trace', 'traces/gcc602_2M.trace', 'traces/mcf605_2M.trace', 'traces/perlbench600_2M.trace']

#jklf

jj = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]
kk = [1, 2, 3, 4]
ll = [6]
ff = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]

total = len(jj) * len(kk) * len(ll) * len(ff)

def run(index):
    curr = 0
    for j in jj:
        for k in kk:
            for l in ll:
                for f in ff:
                    cmd = "./procsim -j %d -k %d -l %d -f %d -t %s" % (j, k, l, f, traces[index])
                    os.system(cmd)
                    curr += 1
                    print("=========  %d out of %d  ========" % (curr, total))


if __name__ == "__main__":
    with Pool(5) as p:
        p.map(run, [0, 1, 2, 3, 4])
