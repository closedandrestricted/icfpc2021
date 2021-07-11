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
bonuses = problem['bonuses']

xs = [xy[0] for xy in hole]
ys = [xy[1] for xy in hole]

plt.plot(xs + [xs[0]], ys + [ys[0]], c='r',
         marker='o', linewidth=5.0, markersize=10)
plt.margins(0.1, 0.1)
plt.gca().invert_yaxis()


def annotate_shift(v):
    return v[0] + 0.25, v[1] - 0.25


for i, v in enumerate(hole):
    plt.annotate(str(i), annotate_shift(v), color='red')


for i, v in enumerate(vertices):
    plt.annotate(str(i), annotate_shift(v), color='green')

for i, e in enumerate(edges):
    x1, y1 = vertices[e[0]]
    x2, y2 = vertices[e[1]]
    plt.plot([x1, x2], [y1, y2], c='g', marker='o',
             linewidth=1.0, markersize=4)

for i, b in enumerate(bonuses):
    x, y = b["position"]
    plt.plot([x,x], [y,y], c='orange', marker='o', linewidth=1.0, markersize=4)
    plt.annotate(b["bonus"], annotate_shift(b["position"]), color="orange")

if args.solution:
    with open(args.solution) as f:
        solution = json.loads(f.read())
    v_solution = solution['vertices']
    for i, e in enumerate(edges):
        x1, y1 = v_solution[e[0]]
        x2, y2 = v_solution[e[1]]
        plt.plot([x1, x2], [y1, y2], c='b', marker='o',
                 linewidth=1.0, markersize=4)

    for i, v in enumerate(v_solution):
        plt.annotate(str(i), annotate_shift(v), color='blue')

plt.show()
