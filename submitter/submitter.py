#!/usr/bin/env python3

# use:
# ./submitter.py --problem 77 --solution ../solutions/feasible/77.json

import argparse
import requests

parser = argparse.ArgumentParser(description='Submit ICFPC 2021 solution.')
parser.add_argument('--problem', type=int, 
                    help='problem number')
parser.add_argument('--solution', type=str,
                    help='path to solution')

args = parser.parse_args()

apiKey = open("../api_key2", "r").read().strip()

with open(args.solution, "r") as fIn:
    params = {"file": fIn}
    response = requests.post("https://poses.live/api/problems/%d/solutions" % args.problem, data=fIn.read(), headers={"Authorization": "Bearer %s" % apiKey})
    print(response.text)
