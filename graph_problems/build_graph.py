from graphviz import Digraph
import json
from pathlib import Path

problems = Path("./problems")

dot = Digraph()
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
                dot.edge(pnum, prob)

dot.render("bonuses.gv")

        

