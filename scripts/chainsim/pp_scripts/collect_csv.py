#!/usr/bin/env python3

import getopt
import re
import os
import sys

def parse_tcpavg(directory):
    xfer_bytes = -999
    xfer_time = -999

    fp = open(directory + os.sep + "tcpavg.txt", 'r')
    for l in fp:
        l = l.strip()
        if len(l) == 0:
            continue
        l = l.split()
        if l[0] == "xfer_time":
            xfer_time = float(l[1])
        elif l[0] == "xfer_bytes":
            xfer_bytes = float(l[1])
    fp.close()

    return (xfer_time, xfer_bytes)

def process_dir(run_dir, dir_data_func):
    for ent in os.listdir(run_dir):
        b = os.path.basename(ent)
        if not re.match("params,", b):
            continue
        l = b.split(',')
        del l[0]
        assert l[-1] == ''
        del l[-1]

        ret = []
        for u in l:
            u = u.split("_", 1)
            ret.append(u[1])
        
        # Process data
        ret.extend(dir_data_func(run_dir + os.sep + ent))

        print(",".join(str(x) for x in ret))

if __name__ == "__main__":
    process_dir(sys.argv[1], parse_tcpavg)  
