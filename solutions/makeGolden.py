#!/usr/bin/env python3

import sys
import os
import argparse
import subprocess
import shutil

sys.path.append("../solver")
import validator

maxProblem = int(open("../max_problem", "r").read().strip())

parser = argparse.ArgumentParser(description='Submit ICFPC 2021 solutions.')
parser.add_argument('--begin', type=int, default=1,
                    help='first problem number')
parser.add_argument('--end', type=int, default=maxProblem,
                    help='last problem number')

args = parser.parse_args()

def validate(problem, solution):
    return validator.validate(problem, solution, False)

    process = subprocess.run(["../solver/validator.py", problem, solution], capture_output=True)
    # print(process)
    if process.returncode == 42:
        return (False, 0)
    else:
        # print(process.stdout.decode())
        # print(process.stderr.decode())
        lines = list(filter(lambda x: x != "", process.stdout.decode().strip().split("\n")))
        return (True, int(lines[-1]))

solutions = ["feasible", "manual", "optimal", "staging", "suboptimal_backtracking", "suboptimal_mcmc", "soptimal", "optimal_with_bonus", "gradient", "webedit", "move", "mctp"]

TMPDST = "golden/goldenDigest.tmp.csv"
digest = open(TMPDST, "w")
print("problem,solution,currentScore,goldenScore", file=digest)

class ProblemState:
    def __init__(self, problemFilename):
        self.bestScore = 1e100
        self.best = ""
        self.bestFilename = ""
        self.problemFilename = problemFilename

    def trySolution(self, solutionFilename):
        if os.path.exists(solutionFilename):
            validationResult = validate(self.problemFilename, solutionFilename)
            if validationResult[0]:
                if validationResult[1] < self.bestScore:
                    self.bestScore = validationResult[1]
                    self.best = s
                    self.bestFilename = solutionFilename
            else:
                print("!!!Validation failed for %s" % solutionFilename)
    

improvements = 0
for problem in range(args.begin, args.end + 1):
    problemFilename = "../problems/%d.json" % problem
    
    state = ProblemState(problemFilename)

    goldenFilename = "golden/%d.json" % problem
    currentGoldenScore = 1e100
    if os.path.exists(goldenFilename):
        goldenValidationResult = validate(problemFilename, goldenFilename)
        if goldenValidationResult[0]:
            currentGoldenScore = goldenValidationResult[1]
        else:
            print("!!!Broken golden result: %s" % goldenFilename)

    for s in solutions:
        solutionFilename = "%s/%d.json" % (s, problem)
        state.trySolution(solutionFilename)

        for mask in range(1, 8):
            solutionFilename = "%s/%d_%d.json" % (s, problem, mask)
            state.trySolution(solutionFilename)

    if state.best:
        print("%s for %d = %d" % (state.best, problem, state.bestScore))
        print("%d,%s,%d,%d" % (problem, state.best, state.bestScore, min(state.bestScore, goldenValidationResult[1])), file=digest)
        if state.bestScore < currentGoldenScore:
            shutil.copyfile(state.bestFilename, goldenFilename)
            improvements += 1
        elif state.bestScore > currentGoldenScore:
            print("!Current golden result is better for %d" % problem)
    else:
        print("%d,none,-1,%d" % (problem, goldenValidationResult[1]), file=digest)

print("Total improvements: %d" % improvements)

digest.close()
shutil.move(TMPDST, "golden/goldenDigest.csv")
