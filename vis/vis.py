#!/usr/bin/python
import argparse
import json
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser()
parser.add_argument('problem')
parser.add_argument('-s', '--solution', required=False)

args = parser.parse_args()

with open(args.problem) as f:
    problem = json.loads(f.read())

hole = problem['hole']
edges = problem['figure']['edges']
vertices = problem['figure']['vertices']

xs = [xy[0] for xy in hole]
ys = [-xy[1] for xy in hole]

plt.plot(xs + [xs[0]], ys + [ys[0]], c='r', marker='o', linewidth=5.0, markersize=10)
plt.margins(0.1, 0.1)
for i, e in enumerate(edges):
    x1, y1 = vertices[e[0]]
    x2, y2 = vertices[e[1]]
    plt.plot([x1, x2], [-y1, -y2], c='g', marker='o', linewidth=1.0, markersize=4)

if args.solution:
    with open(args.solution) as f:
        solution = json.loads(f.read())
    v_solution = solution['vertices']
    for i, e in enumerate(edges):
        x1, y1 = v_solution[e[0]]
        x2, y2 = v_solution[e[1]]
        plt.plot([x1, x2], [-y1, -y2], c='b', marker='o', linewidth=1.0, markersize=4)

plt.show()
