#!/usr/bin/python3
import argparse
import fractions
import json
import sys
import traceback

def validate(argsProblem, argsSolution, debugOutput=True):
    try:
        with open(argsProblem) as f:
            problem = json.loads(f.read())

        with open(argsSolution) as f:
            solution = json.loads(f.read())

        hole = problem['hole']
        epsilon = problem['epsilon']
        edges = problem['figure']['edges']
        p_vertices = problem['figure']['vertices']
        s_vertices = solution['vertices']

        def scale(vertices):
            return[[2 * x[0], 2 * x[1]] for x in vertices]

        # Scale to make it possible to have half-whole numbers to break a leg
        hole = scale(hole)
        epsilon = 4 * epsilon
        p_vertices = scale(p_vertices)
        s_vertices = scale(s_vertices)

        def get_bonus(name):
            if 'bonuses' not in solution:
                return None
            assert len(solution['bonuses']) <= 1
            for bonus in solution['bonuses']:
                if bonus['bonus'] == name:
                    return bonus
            return None

        globalist   = get_bonus('GLOBALIST')
        break_a_leg = get_bonus('BREAK_A_LEG')
        wallhack = get_bonus('WALLHACK')
        superflex = bool(get_bonus('SUPERFLEX'))

        if break_a_leg:
            assert len(s_vertices) == len(p_vertices) + 1
            e = break_a_leg['edge']
            eidx = edges.index(e)
            vidx = len(p_vertices)
            v1idx = edges[eidx][0]
            v2idx = edges[eidx][1]
            v1 = p_vertices[v1idx]
            v2 = p_vertices[v2idx]
            vnew = [(v1[0] + v2[0]) // 2, (v1[1] + v2[1]) // 2]
            p_vertices.append(vnew)
            edges[eidx] = [v1idx, vidx]
            edges.append([v2idx, vidx])

        def sqrdist(p1, p2):
            return (p1[0] - p2[0]) ** 2 + (p1[1] - p2[1]) ** 2

        assert len(s_vertices) == len(p_vertices), "%d != %d" % (len(s_vertices), len(p_vertices))

        # check distances
        MLN = 1000000
        total_penalty = 0

        num_violations = 0
        for e in edges:
            p_sqrd = sqrdist(p_vertices[e[0]], p_vertices[e[1]])
            s_sqrd = sqrdist(s_vertices[e[0]], s_vertices[e[1]])
            penalty = fractions.Fraction(s_sqrd, p_sqrd) - 1
            if globalist:
                total_penalty += abs(penalty)
            elif abs(penalty) > fractions.Fraction(epsilon, MLN):
                if superflex:
                    superflex = False
                else:
                    raise AssertionError(
                        str((abs(penalty), fractions.Fraction(epsilon, MLN))))
        if globalist and total_penalty > fractions.Fraction(epsilon * len(edges), MLN):
            raise AssertionError(
                str((total_penalty, fractions.Fraction(epsilon * len(edges), MLN))))

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

        def try_with_no_vertex(v_wallhack):
            for i in range(len(hole)):
                p1 = hole[i]
                p2 = hole[(i+1) % len(hole)]
                p3 = hole[(i+2) % len(hole)]
                v1 = vec(p2, p1)
                v2 = vec(p2, p3)
                for e in edges:
                    sp1 = s_vertices[e[0]]
                    sp2 = s_vertices[e[1]]
                    if v_wallhack in [sp1, sp2]:
                        continue
                    args = [p1, p2, sp1, sp2]
                    if between(sp1, p2, sp2):
                        vp11 = right_turn(p1, p2, sp1)
                        vp12 = right_turn(p1, p2, sp2)
                        a1 = vp11 and vp12

                        vp21 = right_turn(sp1, p2, p3)
                        vp22 = right_turn(sp2, p2, p3)
                        if right_turn(p1, p2, p3):
                            if not a1:
                                raise AssertionError(
                                    str((p1, p2, p3, sp1, sp2, "between, right")))
                        else:
                            # ??????????????!! ?????? ?????????????????? ?????????????????
                            if not ((vp11 or vp21) and (vp12 or vp22)):
                                raise AssertionError(
                                    str((p1, p2, p3, sp1, sp2, "between, !right")))
                    elif between(sp1, p1, sp2):
                        continue
                    elif touch(*args):
                        vp11 = right_turn(p1, p2, sp1)
                        vp12 = right_turn(p1, p2, sp2)
                        if not vp11 or not vp12:
                            raise AssertionError(str((p1, p2, p3, sp1, sp2, "touch")))
                    elif interesect(*args):
                        raise AssertionError(str((p1, p2, p3, sp1, sp2, "interesect")))
        try:
            try_with_no_vertex(None)
        except Exception:
            if not wallhack:
                raise
            for sv in s_vertices:
                try:
                    try_with_no_vertex(sv)
                    break
                except Exception:
                    if sv is s_vertices[-1]:
                        raise

        collected_bonuses = []
        if 'bonuses' in problem:
            for bonus in problem['bonuses']:
                pos = scale([bonus['position']])[0]
                if any(v == pos for v in s_vertices):
                    collected_bonuses.append(bonus)

        if debugOutput:
            print(json.dumps(collected_bonuses))

        return (True, score() // 4)

    except Exception:
        if debugOutput:
            traceback.print_exc()
        return (False, -1)

if __name__ == "__main__":
# Input arguments:
# problem, solution, bonuses

# Prints to stdout:
# <line with json array with bonus descriptions>
# <line with a single number, score>
# -- or returns error code 42
# Garbage can be printed to stderr.

    parser = argparse.ArgumentParser()
    parser.add_argument('problem')
    parser.add_argument('solution')

    args = parser.parse_args()

    validationResult = validate(args.problem, args.solution)
    if validationResult[0]:
        print(validationResult[1])
    else:
        sys.exit(42)
