from graphviz import Digraph
import json
from pathlib import Path
from collections import defaultdict

problems = Path("./problems")

colors = {
  "BREAK_A_LEG": "red",
  "GLOBALIST": "green",
  "WALLHACK": "yellow",
  "SUPERFLEX": "orange",
}

dot = Digraph()

available_bonuses = defaultdict(list)

for p in problems.iterdir():
    if not p.name.endswith(".json"): continue
    pnum = p.name.split(".")[0]
    dot.node(pnum)
    with open(p, "rt") as f:
        js = json.load(f)
        if "bonuses" in js:
            for bonus in js["bonuses"]:
                btype = bonus["bonus"]
                prob = str(bonus["problem"])
                dot.edge(pnum, prob, color = colors[btype], label=btype[0])
                available_bonuses[int(prob)].append((pnum, btype))

for k, v in sorted(available_bonuses.items()):
    print(f"Bonuses available for problem {k}:")
    for pnum, btype in v:
        print(f"  - from problem {pnum} - {btype}")

#dot.render("bonuses.gv")

        

