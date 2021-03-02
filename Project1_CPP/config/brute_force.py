import os
import sys
import subprocess
import threading
from tqdm import tqdm
from multiprocessing import Process
from multiprocessing import Pool


def get_filename(c, b, s, rp, v):
    return "experimental/experiment_c{}_b{}_s{}_{}_v{}.conf".format(c, b, s, rp, v)


def write_conf(c, b, s, rp, v):
    f = open("experimental/experiment_c{}_b{}_s{}_{}_v{}.conf".format(c, b, s, rp, v), "w")
    f.write("{\n\"L1 Data\" : {\n")
    f.write("\"C\" : {},\n".format(c))
    f.write("\"B\" : {},\n".format(b))
    f.write("\"S\" : {},\n".format(s))
    f.write("},\n")
    f.write("\"Replacement Policy\" : \"{}\",\n".format(rp))
    f.write("\"Victim Cache\" : {\n")
    f.write("\"V\" : {}".format(v))
    f.write("}\n}")
    f.close()


c_range = [9, 10, 11, 12, 13, 14, 15]
b_range = [1, 2, 3, 4, 5, 6]
s_range = [0, 1, 2, 3, 4]
rp_range = ["LRU", "FIFO", "LFU"]
config = []
i = 0

def generate_configs():
    global i
    for c in c_range:
        for b in b_range:
            for s in s_range:
                for rp in rp_range:
                    write_conf(c, b, s, rp, 4)
                    i += 1
                if s == s_range[-1]:
                    for rp in rp_range:
                        write_conf(c, b, c - b, rp, 4)
                        i += 1
    print("total configs", i)

def run1(type):
    configs = config[0 : 149]
    for i in tqdm(range(0, 149)):
        conf = configs[i]
        if type == 0:
            cmd = "../build/cachesim -c experimental/{} -i ../experimental/mm_basic_192.trace".format(conf)
        else:
            cmd = "../build/cachesim -c experimental/{} -i ../experimental/mm_tiled_192_4.trace".format(conf)

        # cmd = "../build/cachesim -c {} -i ../experimental/mm_basic_192.trace".format(conf)
        print(" === " + cmd)
        os.system(cmd)

def run2(type):
    configs = config[149 : 149 * 2]
    for conf in configs:
        if type == 0:
            cmd = "../build/cachesim -c experimental/{} -i ../experimental/mm_basic_192.trace".format(conf)
        else:
            cmd = "../build/cachesim -c experimental/{} -i ../experimental/mm_tiled_192_4.trace".format(conf)

        # cmd = "../build/cachesim -c {} -i ../experimental/mm_basic_192.trace".format(conf)
        print(" === " + cmd)
        os.system(cmd)


def run3(type):
    configs = config[149 * 2 : 149 * 3]
    for conf in configs:
        if type == 0:
            cmd = "../build/cachesim -c experimental/{} -i ../experimental/mm_basic_192.trace".format(conf)
        else:
            cmd = "../build/cachesim -c experimental/{} -i ../experimental/mm_tiled_192_4.trace".format(conf)

        # cmd = "../build/cachesim -c {} -i ../experimental/mm_basic_192.trace".format(conf)
        print(" === " + cmd)
        os.system(cmd)


def run4(type):
    configs = config[149 *  3 : 149 * 4]
    for conf in configs:
        if type == 0:
            cmd = "../build/cachesim -c experimental/{} -i ../experimental/mm_basic_192.trace".format(conf)
        else:
            cmd = "../build/cachesim -c experimental/{} -i ../experimental/mm_tiled_192_4.trace".format(conf)

        # cmd = "../build/cachesim -c {} -i ../experimental/mm_basic_192.trace".format(conf)
        print(" === " + cmd)
        os.system(cmd)


def run5(type):
    configs = config[149 * 4 :]
    for conf in configs:
        if type == 0:
            cmd = "../build/cachesim -c experimental/{} -i ../experimental/mm_basic_192.trace".format(conf)
        else:
            cmd = "../build/cachesim -c experimental/{} -i ../experimental/mm_tiled_192_4.trace".format(conf)

        # cmd = "../build/cachesim -c {} -i ../experimental/mm_basic_192.trace".format(conf)
        print(" === " + cmd)
        os.system(cmd)



if __name__ == "__main__":
    # generate_configs()

    # for s in s_range:
    #     filename = get_filename(15, 6, s, "LRU", 4)
    #     run(filename)

    # # generate_configs();
    config = [f for f in os.listdir("experimental/")]
    print(len(config))

    t1 = threading.Thread(target=run1, args=(0, ))
    t2 = threading.Thread(target=run2, args=(0, ))
    t3 = threading.Thread(target=run3, args=(0, ))
    t4 = threading.Thread(target=run4, args=(0, ))
    t5 = threading.Thread(target=run5, args=(0, ))

    t1.start()
    t2.start()
    t3.start()
    t4.start()
    t5.start()
    
    t1.join()
    t2.join()
    t3.join()
    t4.join()
    t5.join()


    os.system("cp -r ../summaries/ ../summaries_basic/")
    os.system("rm ../summaries/*")

    # for i in tqdm(range(int(735 / 5))):
    #     with Pool(5) as p:
    #         p.map(run2, [config[i*8], config[i*8+1], config[i*8+2], config[i*8+3], config[i*8+4]])


    t1 = threading.Thread(target=run1, args=(1, ))
    t2 = threading.Thread(target=run2, args=(1, ))
    t3 = threading.Thread(target=run3, args=(1, ))
    t4 = threading.Thread(target=run4, args=(1, ))
    t5 = threading.Thread(target=run5, args=(1, ))

    t1.start()
    t2.start()
    t3.start()
    t4.start()
    t5.start()
    
    t1.join()
    t2.join()
    t3.join()
    t4.join()
    t5.join()
