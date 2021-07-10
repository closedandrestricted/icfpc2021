#!/usr/bin/env python

import os
import argparse
import subprocess

parser = argparse.ArgumentParser(description='Submit ICFPC 2021 solutions.')
parser.add_argument('--begin', type=int, default=1,
                    help='first problem number')
parser.add_argument('--end', type=int, default=78,
                    help='last problem number')
parser.add_argument("--path", type=str)

args = parser.parse_args()

for problem in range(args.begin, args.end + 1):
    filename = "%s/%d.json" % (args.path, problem)
    if os.path.exists(filename):
        out = subprocess.check_output(["./submitter.py", "--problem", str(problem), "--solution", filename])
        print("%d %s" % (problem, out.strip()))
    else:
        print("Solution for %d not found" % problem)
