#!/usr/bin/env python

import json
import pandas
import math
import os

this_dir = os.path.dirname(os.path.abspath(__file__))

tabltGoldenScores = pandas.read_csv("../solutions/golden/goldenDigest.csv")
goldenScores = {}
for idx, row in tabltGoldenScores.iterrows():
    goldenScores[int(idx) + 1] = row['goldenScore']

scores = pandas.read_csv("scores.csv")
print(scores)
total_gap = 0
total_gap_golden = 0
for idx, row in scores.iterrows():
    with open(os.path.join(this_dir, '../problems/%d.json' % row['problem'])) as f:
        problem = json.loads(f.read())

    def problem_score(player_score):
        le = len(problem['figure']['edges'])
        lh = len(problem['hole'])
        lv = len(problem['figure']['vertices'])
        a = 1000 * math.log2(le * lh * lv / 6)
        b = math.sqrt((1. + row['best_score']) / (1. + player_score))
        if player_score != player_score:
            return 0
        return int(math.ceil(a * b))

    now = problem_score(row['score'])
    nowGolden = problem_score(goldenScores[int(row['problem'])])
    best = problem_score(row['best_score'])
    gap = best - now
    gapGolden = best - nowGolden
    print(int(row['problem']), gap, now, nowGolden - now)
    total_gap += gap
    total_gap_golden += gapGolden

print('TOTAL = %d' % total_gap)
print('TOTAL_GOLDEN = %d' % total_gap_golden)
