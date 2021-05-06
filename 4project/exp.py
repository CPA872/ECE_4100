import os
import subprocess

traces = [
            'traces/experiment1',
            'traces/experiment2',
            'traces/experiment3',
            'traces/experiment4',
            'traces/experiment5',
            'traces/experiment6',
            'traces/experiment7',
            'traces/experiment8',
        ]
protocols = ['MI', "MSI", "MESI", "MOSI", "MOESI", "MOESIF"]

i = 0
for t in traces:
    for p in protocols:
        i += 1
        filename = "logs/" + t[7:] + "_" + p + ".log"
        cmd = "./sim_trace -t " + t + " -p " + p + " &> " + filename
        print(cmd)

