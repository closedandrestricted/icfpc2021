#!/usr/bin/python3
import argparse
import json
import os
import subprocess
import sys

this_dir = os.path.dirname(os.path.abspath(__file__))

parser = argparse.ArgumentParser()
parser.add_argument('problems')
parser.add_argument('solutions')


args = parser.parse_args()


def indexes(path):
    indexes = []
    for path in os.listdir(path):
        if path.endswith('.json'):
            try:
                indexes.append(int(path.split('.')[0]))
            except:
                pass

    return sorted(indexes)


problems = indexes(args.problems)
solutions = set(indexes(args.solutions))


for p in problems:
    if p not in solutions:
        continue
    p_path = os.path.join(args.problems, str(p) + '.json')
    s_path = os.path.join(args.solutions, str(p) + '.json')
    try:
        cmdargs = [
            os.path.join(this_dir, 'validator.py'),
            p_path,
            s_path,
        ]
        output = subprocess.check_output(
            ' '.join(cmdargs), shell=True, stderr=subprocess.PIPE)
        print('%d %d' % (p, int(output.decode())))
    except subprocess.SubprocessError:
        print('%d FAILED' % p, file=sys.stderr)
