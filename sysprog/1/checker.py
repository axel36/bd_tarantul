import random
import argparse
import sys

maxint = 1 << 31

parser = argparse.ArgumentParser(description = "Check that a file contains "\
                           "not decreasing sequence of "\
                           "numbers")
parser.add_argument('-f', type=str, required=True, help="file name")
args = parser.parse_args()


f = open(args.f, 'r')
data = f.read()
f.close()
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
