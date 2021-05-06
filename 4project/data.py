import re
import os
import pathlib

results = os.listdir("logs/")
f_final = open("summary.csv", "w")
f_final.write("experiment N, " + "protocol, " +  "run_time, "+ "misses, "+ "accesses, "+ "upgrades, "+ "transfers\n")

for result in results:
    f_path = "logs/" + result
    f = open(f_path)
    for line in f.readlines():
        if re.search("Run Time", line):
            run_time = line.split()[2]
        if re.search("Misses", line):
            misses = line.split()[2]
        if re.search("Accesses", line):
            accesses = line.split()[2]
        if re.search("Upgrades", line):
            upgrades = line.split()[2]
        if re.search("Transfers", line):
            transfers = line.split()[2]
    
    to_write = result[:11] + "," + result[12:] +", "+ run_time+", "+ misses+", "+ accesses+", "+ upgrades+", "+ transfers
    print(to_write)
    f_final.write(to_write)
    f_final.write("\n")
