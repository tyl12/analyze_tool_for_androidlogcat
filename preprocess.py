#!/usr/bin/python

import os
import sys
import getopt
import re
import datetime
import shutil

def calcute_diff_time(lines):
    startline=lines[0]
    endline=lines[1]
    start_time=re.findall("(\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3})",startline)
    end_time=re.findall("(\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3})",endline)
    start_time_decode=datetime.datetime.strptime(start_time[0], "%m-%d %H:%M:%S.%f")
    end_time_decode=datetime.datetime.strptime(end_time[0], "%m-%d %H:%M:%S.%f")
    delta_time = end_time_decode-start_time_decode
    #print(delta_time.microseconds)
    #print("delta_time=" + delta_time.microseconds)
    return  delta_time.days*24*60*60*60 + delta_time.seconds*1000 + delta_time.microseconds/1000



opts, args = getopt.getopt(sys.argv[1:], "s:f:d")
input_file=sys.stdin
split_mark='='
debug_mode=False
for op, value in opts:
    if op == "-s":
        pattern_marks = value
    elif op == "-f":
        input_file = open(value)
    elif op == "-d":
        debug_mode=True

s=(input_file == sys.stdin) and "stdin" or str(input_file)
print("--> file: " + s)
print("--> split: " + pattern_marks)
print("--> debug mode: " + str(debug_mode))
print("\n")


tmpdir="./intermediate"
if os.path.exists(tmpdir):
    shutil.rmtree(tmpdir)
os.mkdir(tmpdir)

pattern_mark_list=pattern_marks.split("@")

#workaround start: for simplicity, temporarily we handle only two patterns :(
print(pattern_mark_list)
if ( len(pattern_mark_list) != 2):
    print("ERROR: pattern size check failed")

absolute_parcel_list=[]
diff_parcel_list=[]

parcel_state=0;
parcel_state_cnt=len(pattern_mark_list)
parcel_cnt=0;


diff_time_tag=pattern_mark_list[0] + " => " + pattern_mark_list[1] + " @ "

filter_file=open(os.path.join(tmpdir, "_filterfile"), "w")
timediff_file=open(os.path.join(tmpdir, "_timedifffile"), "w")

parcel_lines=[]
for line in input_file:
    if (pattern_mark_list[parcel_state] in line):
        parcel_lines.append(line)
        parcel_state=(parcel_state+1)%parcel_state_cnt
        if (parcel_state == 0):
            filter_file.write(str(parcel_lines));
            filter_file.write("\n");

            if debug_mode:
                [print(t) for t in parcel_lines]

            #for i in parcel_lines:
            #    filter_file.write(i);

            diff_time = calcute_diff_time(parcel_lines)
            diff_time_line=diff_time_tag + str(diff_time);
            timediff_file.write(diff_time_line)
            timediff_file.write("\n")
            parcel_lines=[]  #reset to default

filter_file.close()
timediff_file.close()

#os.popen("python ./Analyze.py -f " + str(os.path.join(tmpdir, "_timedifffile")) + " -s " + "\":\"" + " -d ")

