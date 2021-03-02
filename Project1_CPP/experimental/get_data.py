import os
import re

base = "summaries_basic/"

f = open("mm_basic.csv", "w")
f.write("mm_basic\nc, b, s, rp, v, AAT\n")
counter = 0
configs = [i for i in os.listdir(base)]

for config in configs:
    counter += 1
    print(counter)
    result = open(base + config)
    config_list = config.split("_")
    print(config_list)
    
    c = int(config_list[0][1:])
    b = int(config_list[1][1:])
    s = int(config_list[2][1:])
    rp = config_list[3]
    v = 4

    for line in result.readlines():
        if re.search("Average Access Time", line):
            AAT = float(line.split(" ")[-1])
    result.close()

    f.write("%d, %d, %d, %s, %d, %f\n" % (c, b, s, rp, v, AAT))

f.close()

