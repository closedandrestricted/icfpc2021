import json
import numpy as np

with open('problems/65.json') as f:
    p = json.load(f)

def dist2(p, q):
    dx = p[0] - q[0]
    dy = p[1] - q[1]
    return dx * dx + dy * dy

hole = p['hole']
eps = p['epsilon']
vs = p['figure']['vertices']
es = p['figure']['edges']

def match(a, b, u, v):
    d2 = dist2(a, b)
    return abs(dist2(vs[u], vs[v]) - d2) * 1000000 <= eps * d2

# for i in range(len(hole)):
#     d2 = dist2(hole[i], hole[(i + 1) % len(hole)])
#     for e in es:
#         u, v = e
#         if abs(dist2(vs[u], vs[v]) - d2) * 1000000 <= eps * d2:
#             print(i, e)

g = np.zeros([len(vs), len(vs)], dtype=np.int32)
for e in es:
    u, v = e
    g[u, v] += 1
    g[v, u] += 1

quads = []
for a in range(len(vs)):
    for b in range(len(vs)):
        for c in range(len(vs)):
            if g[a, b] + g[a, c] + g[b, c] == 0:
                continue
            for d in range(len(vs)):
                s = g[a, b] + g[a, c] + g[a, d] + g[b, c] + g[b, d] + g[c, d]
                if len(set([a, b, c, d])) != 4 or s < 4:
                    continue
                quads.append([a, b, c, d])
# print(quads)

for i in range(len(hole)):
    print(f"{i}:")
    rng = [hole[(i + j) % len(hole)] for j in range(4)]
    for q in quads:
        good = True
        for x in range(4):
            for y in range(x + 1, 4):
                good &= g[q[x], q[y]] == 0 or match(rng[x], rng[y], q[x], q[y])
        if good:
            print(q)