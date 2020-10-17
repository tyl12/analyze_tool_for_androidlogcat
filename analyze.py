#!/usr/bin/python

import sys
import getopt
import re

opts, args = getopt.getopt(sys.argv[1:], "s:f:d")
input_file=sys.stdin
split_mark='='
debug_mode=False
for op, value in opts:
    if op == "-s":
        split_mark = value
    elif op == "-f":
        input_file = open(value)
    elif op == "-d":
        debug_mode=True

s=(input_file == sys.stdin) and "stdin" or str(input_file)
print("--> file: " + s)
print("--> split: " + split_mark)
print("--> debug mode: " + str(debug_mode))
print("\n")

data=[]
for line in input_file:
    val = line.split(split_mark)[-1]
    #print(line)
    #print(val)

    #pat = re.match('.*=\s*(\d+).*', line)

    s_re=r'.*'+split_mark+r'\s*([+-]?\d+).*'
    pat = re.match(s_re, line)
    #print(pat)
    if pat:
        val = pat.groups()[0]
        if val is not None:
            data.append(int(val))
            if debug_mode:
                print(line.strip() + "  =>  " + val)
if data:

    minval=min(data)
    maxval=max(data)

    step_cnt = 10
    distribute = [0]*(step_cnt+1)

    bin_step=(maxval-minval)/(1.0*step_cnt)
    if (bin_step > 1E-10):
        for d in data:
            index=int((d-minval)/bin_step)
            distribute[index] += 1

    print("\n")
    print("Data Min:\t " + str(minval))
    print("Data Max:\t " + str(maxval))
    print("Data step cnt: \t " + str(step_cnt))
    print("Data step len: \t " + str(bin_step))
    print("Data Distribu: \t " + str(distribute))

    print("Data Sum:\t " + str(sum(data)))
    print("Data count:\t " + str(len(data)))
    print("Data Avg: \t " + str(sum(data)*1.0/len(data)))

else:
    print("No data available!")

