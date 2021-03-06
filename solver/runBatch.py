#!/usr/bin/env python3

import os
import argparse
import subprocess
import json

maxProblem = int(open("./max_problem", "r").read().strip())

parser = argparse.ArgumentParser(description='Submit ICFPC 2021 solutions.')
parser.add_argument('--begin', type=int, default=1,
                    help='first problem number')
parser.add_argument('--end', type=int, default=maxProblem,
                    help='last problem number')
parser.add_argument("--path", type=str)
parser.add_argument('--override', action="store_true",
                    help='submit unconditionally')
parser.add_argument('--step', type=int, default=1,
                    help='for step')
parser.add_argument('--mcmc', action="store_true",
                    help='run MCMC solver')
parser.add_argument('--timeout', type=int, default=60,
                    help='run timeout')
parser.add_argument('--init', action="store_true",
                    help='init')

args = parser.parse_args()

for problem in range(args.begin, args.end + 1, args.step):
    try:
        subprocess.check_output(["./bazel-bin/solver/cli", "-test-idx", str(problem), "-alex" if not args.mcmc else ""] + (["-init", "solutions/golden/%d.json" % problem] if args.init else []), timeout=args.timeout)
    except:
        pass
