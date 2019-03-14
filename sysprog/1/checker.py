import random
import argparse
import sys
import subprocess
import time


maxint = 1 << 31

# parser = argparse.ArgumentParser(description = "Check that a file contains "\
#                            "not decreasing sequence of "\
#                            "numbers")
# parser.add_argument('-f', type=str, required=True, help="file name")
# args = parser.parse_args()

N = 6
names = []
process = subprocess.Popen("gcc hw_sort.cpp -o exehw.out".split(), stdout=subprocess.PIPE)
time.sleep(1)
for i in range(N):
    process = subprocess.Popen("python3 generator.py -f test{}.txt -c 10000 -m 10000".format(i).split(),
                               stdout=subprocess.PIPE)
    names.append("test"+str(i))

process = subprocess.Popen("python3 generator.py -f test{}.txt -c 10000 -m 100000".format(N+1).split(),
                           stdout=subprocess.PIPE)

with open("out.txt", 'r') as f:
    data = f.read()

is_ok = True
data = data.split()
prev_number = -(1 << 31 - 1)
for i in range(0, len(data)):
    try:
        v = int(data[i])
        if v < prev_number:
            is_ok = False
            print('Error on numbers {} {}, position in file - {}'.format(prev_number, v, i))
        prev_number = v
    except:
        pass
if is_ok:
    print('All is ok')
