import urllib.request
import argparse
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("--max_problem", "-m", default=88)

args = parser.parse_args()

new_dir = Path("./problems")
new_dir.mkdir(exist_ok=True, parents=True)

def download_one_problem(i, path):
    req = urllib.request.urlopen(f"https://poses.live/problems/{i}/download?")
    assert req.status == 200
    lines = req.readlines()
    assert len(lines) == 1
    with open(path.joinpath(f"{i}.json"), "wt") as f:
        f.write(lines[0].decode("utf-8"))

for i in range(1, args.max_problem+1):
    print(f"Loading problem {i}")
    download_one_problem(i, new_dir)
