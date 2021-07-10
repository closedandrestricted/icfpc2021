#!/usr/bin/env python3

import os
import argparse
import subprocess
import shutil

parser = argparse.ArgumentParser(description='Submit ICFPC 2021 solutions.')
parser.add_argument('--begin', type=int, default=1,
                    help='first problem number')
parser.add_argument('--end', type=int, default=78,
                    help='last problem number')

args = parser.parse_args()

solutions = ["manual", "feasible"]

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

solutions = ["feasible", "manual", "optimial", "suboptimal_backtracking"]

for problem in range(args.begin, args.end + 1):
    bestScore = 1e100
    best = ""
    bestFilename = ""
    for s in solutions:
        solutionFilename = "%s/%d.json" % (s, problem)
        if os.path.exists(solutionFilename):
            validationResult = validate("../problems/%d.json" % problem, solutionFilename)
            if validationResult[0]:
                if validationResult[1] < bestScore:
                    bestScore = validationResult[1]
                    best = s
                    bestFilename = solutionFilename
    if best:
        print("%s for %d = %d" % (best, problem, bestScore))
        shutil.copyfile(bestFilename, "golden/%d.json" % problem)


