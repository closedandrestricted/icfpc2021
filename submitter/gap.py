#!/usr/bin/env python

import pandas
import math

scores = pandas.read_csv("scores.csv")
print(scores)
for idx, row in scores.iterrows():
    print(idx, row['score']*math.sqrt(row['score']/row['best_score']))
