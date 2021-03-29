import os
import re 

gcc = open("gcc.csv", "w")
mcf = open("mcf.csv", "w")
perl = open("perl.csv", "w")
x264 = open("x264.csv", "w")


gcc.write("o,g,p,trace,accuracy\n")
mcf.write("o,g,p,trace,accuracy\n")
perl.write("o,g,p,trace,accuracy\n")
x264.write("o,g,p,trace,accuracy\n")

files = os.listdir("data")
for f in files:
    cur_file = "data/" + f
    o, g, p, trace = cur_file[5:].split("_")
    o = o[1:]
    g = g[1:]
    p = p[1:]
    outfile = open(cur_file, "r")
    accuracy = 0
    for line in outfile.readlines():
        if re.search("accuracy", line):
            accuracy = line.split(" ")[-1]


    if "gcc" in cur_file:
        gcc.write("%d, %d, %d, %s, %f\n" % (int(o), int(g), int(p), trace, float(accuracy)))
    
    if "mcf" in cur_file:
        mcf.write("%d, %d, %d, %s, %f\n" % (int(o), int(g), int(p), trace, float(accuracy)))
    
        
    if "perl" in cur_file:
        perl.write("%d, %d, %d, %s, %f\n" % (int(o), int(g), int(p), trace, float(accuracy)))
    
    if "x264" in cur_file:
        x264.write("%d, %d, %d, %s, %f\n" % (int(o), int(g), int(p), trace, float(accuracy)))
        

gcc.close()
mcf.close()
perl.close()
x264.close()
