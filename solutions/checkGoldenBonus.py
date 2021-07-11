#!/usr/bin/env python3

from pathlib import Path
import json

probs = Path("../problems")
for p in Path("../solutions/golden/").iterdir():
    if not p.name.endswith(".json"): continue
    js = json.load(open(probs.joinpath(p.name), "rt"))
    if not "bonuses" in js: continue
    vertices = set((x,y) for x,y in json.load(open(p, "rt"))["vertices"])
    for bonus in js["bonuses"]:
        pos = bonus["position"]
        bonusfor = bonus["problem"]
        btype = bonus["bonus"]
        if tuple(pos) in vertices:
            print(f"Got bonus in {p.name} for {bonusfor}: {btype}")


