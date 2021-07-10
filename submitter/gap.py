#!/usr/bin/env python

import json
import pandas
import math
import os

this_dir = os.path.dirname(os.path.abspath(__file__))

scores = pandas.read_csv("scores.csv")
print(scores)
total_gap = 0
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

    gap = problem_score(row['best_score']) - problem_score(row['score'])
    print(int(row['problem']), gap)
    total_gap += gap

print('TOTAL = ', total_gap)
