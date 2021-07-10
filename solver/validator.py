#!/usr/bin/python3
import argparse
import json
import sys

parser = argparse.ArgumentParser()
parser.add_argument('problem')
parser.add_argument('solution')


args = parser.parse_args()

with open(args.problem) as f:
    problem = json.loads(f.read())

with open(args.solution) as f:
    solution = json.loads(f.read())


hole = problem['hole']
epsilon = problem['epsilon']
edges = problem['figure']['edges']
p_vertices = problem['figure']['vertices']
s_vertices = solution['vertices']


def sqrdist(p1, p2):
    return (p1[0] - p2[0]) ** 2 + (p1[1] - p2[1]) ** 2


# check distances
MLN = 1000000
for e in edges:
    p_sqrd = sqrdist(p_vertices[e[0]], p_vertices[e[1]])
    s_sqrd = sqrdist(s_vertices[e[0]], s_vertices[e[1]])
    if s_sqrd > p_sqrd:
        if s_sqrd * MLN - p_sqrd * MLN > epsilon * p_sqrd:
            sys.exit(42)
    if p_sqrd < s_sqrd:
        if p_sqrd * MLN - s_sqrd * MLN > epsilon * p_sqrd:
            sys.exit(42)
    print(p_sqrd, s_sqrd, epsilon, p_sqrd *
          MLN - s_sqrd * MLN, epsilon * p_sqrd, file=sys.stderr)


def score():
    s = 0
    for h in hole:
        s1 = None
        for v in s_vertices:
            d = sqrdist(h, v)
            if s1 is None or d < s1:
                s1 = d
        s += s1
    return s


def vec(p1, p2):  # from p1 to p2
    return p2[0] - p1[0], p2[1] - p1[1]


def vprod(v1, v2):
    return v1[0] * v2[1] - v1[1] * v2[0]


def sprod(v1, v2):
    return v1[0] * v2[0] + v1[1] * v2[1]


def itersect_stats(p1b, p1e, p2b, p2e):
    v1 = vec(p1e, p1b)
    v2 = vec(p2e, p2b)
    mul1 = vprod(v1, vec(p2b, p1b)) * vprod(v1, vec(p2e, p1b))
    mul2 = vprod(v2, vec(p1b, p2b)) * vprod(v2, vec(p1e, p2b))
    return mul1, mul2


def interesect(p1b, p1e, p2b, p2e):
    mul1, mul2 = itersect_stats(p1b, p1e, p2b, p2e)
    return mul1 <= 0 and mul2 <= 0


def between(p1, pm, p2):
    v1 = vec(pm, p1)
    v2 = vec(pm, p2)
    return vprod(v1, v2) == 0 and sprod(v1, v2) <= 0


assert between([0, 0], [1, 0], [2, 0])
assert between([0, 0], [1, 1], [2, 2])
assert between([0, 0], [0, 0], [2, 2])
assert not between([0, 0], [-1, -1], [2, 2])

def touch(p1b, p1e, p2b, p2e):
    mul1, mul2 = itersect_stats(p1b, p1e, p2b, p2e)
    return (mul1 == 0 and mul2 <= 0) or (mul2 == 0 and mul1 <= 0)


def touch_semiopen(p1b_excl, p1e, p2b, p2e):
    if not between(p2b, p1b_excl, p2e):
        return False
    v1 = vec(p1b_excl, p1e)
    v2 = vec(p2e, p2b)
    if vprod(v1, v2) != 0:
        return True
    return not between(p2b, p1e, p2e)


assert touch_semiopen([1, 1], [0, 0], [0, 2], [2, 0])

assert interesect([1, 1], [3, 3], [1, 3], [3, 1])
assert not touch([1, 1], [3, 3], [1, 3], [3, 1])

assert not interesect([1, 3], [3, 3], [1, 1], [3, 1])
assert not touch([1, 3], [3, 3], [1, 1], [3, 1])

assert interesect([1, 1], [3, 3], [2, 2], [3, 1])
assert touch([1, 1], [3, 3], [2, 2], [3, 1])


def right_turn(p1, pm, p3):
    v1 = vec(pm, p1)
    v2 = vec(pm, p3)
    return vprod(v1, v2) <= 0


assert right_turn([10, 5], [10, 10], [0, 0])
assert right_turn([10, 10], [10, 5], [12, 4])


for i in range(len(hole)):
    p1 = hole[i]
    p2 = hole[(i+1) % len(hole)]
    p3 = hole[(i+2) % len(hole)]
    v1 = vec(p2, p1)
    v2 = vec(p2, p3)
    for e in edges:
        sp1 = s_vertices[e[0]]
        sp2 = s_vertices[e[1]]
        args = [p1, p2, sp1, sp2]
        if between(sp1, p2, sp2):
            vp11 = right_turn(p1, p2, sp1)
            vp12 = right_turn(p1, p2, sp2)
            a1 = vp11 and vp12

            vp21 = right_turn(sp1, p2, p3)
            vp22 = right_turn(sp2, p2, p3)
            if right_turn(p1, p2, p3):
                if not a1:
                    print(p1, p2, p3, sp1, sp2, "between, right", file=sys.stderr)
                    sys.exit(42)
            else:
                if not ((vp11 or vp21) and (vp12 or vp22)):  # КОСТЫЛЬ!! Как нормально написать?
                    print(p1, p2, p3, sp1, sp2, "between, !right", file=sys.stderr)
                    sys.exit(42)
        elif between(sp1, p1, sp2):
            continue
        elif touch(*args):
            vp11 = right_turn(p1, p2, sp1)
            vp12 = right_turn(p1, p2, sp2)
            if not vp11 or not vp12:
                print(p1, p2, p3, sp1, sp2, "touch", file=sys.stderr)
                sys.exit(42)
        elif interesect(*args):
            print(p1, p2, p3, sp1, sp2, "interesect", file=sys.stderr)
            print(*args, file=sys.stderr)
            sys.exit(42)


print(score())
