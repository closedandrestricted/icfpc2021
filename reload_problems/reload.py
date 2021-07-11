#!/usr/bin/env python3

import urllib.request
import sys
import argparse
from pathlib import Path

new_dir = Path("./problems")
new_dir.mkdir(exist_ok=True, parents=True)

def download_one_problem(i, path):
    req = urllib.request.urlopen(f"https://poses.live/problems/{i}/download?")
    assert req.status == 200
    lines = req.readlines()
    assert len(lines) == 1
    with open(path.joinpath(f"{i}.json"), "wt") as f:
        f.write(lines[0].decode("utf-8"))

i = 1
while True:
    print(f"Loading problem {i}")
    try:
        download_one_problem(i, new_dir)
    except urllib.error.HTTPError as e:
        print(f"There is no problem {i} - looks like there are only {i-1} problems")
        sys.exit(0)
    i += 1
