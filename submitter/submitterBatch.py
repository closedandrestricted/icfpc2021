#!/usr/bin/env python

# Use:
# ./submitterBatch.py --path ../solutions/golden

import os
import argparse
import subprocess
import json

parser = argparse.ArgumentParser(description='Submit ICFPC 2021 solutions.')
parser.add_argument('--begin', type=int, default=1,
                    help='first problem number')
parser.add_argument('--end', type=int, default=88,
                    help='last problem number')
parser.add_argument("--path", type=str)
parser.add_argument('--override', action="store_true",
                    help='submit unconditionally')

args = parser.parse_args()

def validate(problem, solution):
    process = subprocess.run(["../solver/validator.py", problem, solution], capture_output=True)
    # print(process)
    if process.returncode == 42:
        return (False, 0)
    else:
        # print(process.stdout.decode())
        # print(process.stderr.decode())
        lines = list(filter(lambda x: x != "", process.stdout.decode().strip().split("\n")))
        return (True, int(lines[-1]))

STATUS_FILENAME = "submitter_status.json"

status = {}
if os.path.exists(STATUS_FILENAME):
    with open(STATUS_FILENAME, "r") as fIn:
        status = json.load(fIn)

for problem in range(args.begin, args.end + 1):
    problemFilename = "../problems/%d.json" % problem
    filename = "%s/%d.json" % (args.path, problem)
    if os.path.exists(filename):
        validationResult = validate(problemFilename, filename)
        if validationResult[0]:
            sProblem = str(problem)
            if args.override or (sProblem not in status) or (validationResult[1] < status[sProblem]):
                out = subprocess.check_output(["./submitter.py", "--problem", str(problem), "--solution", filename])
                status[problem] = validationResult[1]
                print("%d %s" % (problem, out.strip()))
        else:
            print("!!!Validation failed for %s" % filename)
    else:
        print("Solution for %d not found" % problem)

with open(STATUS_FILENAME, "w") as fOut:
    json.dump(status, fOut, indent=4)
