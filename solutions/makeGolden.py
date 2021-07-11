#!/usr/bin/env python3

import os
import argparse
import subprocess
import shutil

maxProblem = int(open("../max_problem", "r").read().strip())

parser = argparse.ArgumentParser(description='Submit ICFPC 2021 solutions.')
parser.add_argument('--begin', type=int, default=1,
                    help='first problem number')
parser.add_argument('--end', type=int, default=maxProblem,
                    help='last problem number')

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

solutions = ["feasible", "manual", "optimal", "staging", "suboptimal_backtracking", "suboptimal_mcmc", "soptimal", "optimal_with_bonus"]

digest = open("golden/goldenDigest.csv", "w")

for problem in range(args.begin, args.end + 1):
    problemFilename = "../problems/%d.json" % problem
    goldenFilename = "golden/%d.json" % problem
    currentGoldenScore = 1e100
    if os.path.exists(goldenFilename):
        goldenValidationResult = validate(problemFilename, goldenFilename)
        if goldenValidationResult[0]:
            currentGoldenScore = goldenValidationResult[1]
        else:
            print("!!!Broken golden result: %s" % goldenFilename)

    bestScore = 1e100
    best = ""
    bestFilename = ""
    for s in solutions:
        solutionFilename = "%s/%d.json" % (s, problem)
        if os.path.exists(solutionFilename):
            validationResult = validate(problemFilename, solutionFilename)
            if validationResult[0]:
                if validationResult[1] < bestScore:
                    bestScore = validationResult[1]
                    best = s
                    bestFilename = solutionFilename
    if best:
        print("%s for %d = %d" % (best, problem, bestScore))
        print("%d,%s,%d" % (problem, best, bestScore), file=digest)
        if bestScore <= currentGoldenScore:
            shutil.copyfile(bestFilename, goldenFilename)
        else:
            print("!Current golden result is better for %d" % problem)

digest.close()
