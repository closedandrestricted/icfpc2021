#!/usr/bin/env python3

import os
import requests
import re
import subprocess
import json

this_dir = os.path.dirname(os.path.abspath(__file__))

with open("submitter_status.json", "r") as fIn:
    status = json.load(fIn)

with requests.Session() as session:
    r = session.post("https://poses.live/login", data={"login.email": "denboxarchive@gmail.com", "login.password": "B0ltzMaNn"})
    # print(r.text)
    
    with open("scores.csv", "w") as fOut:
        print("problem,score,best_score", file=fOut)
        matchTrs = re.findall(r'<tr>(.*?)</tr>', r.text)
        for index, tr in enumerate(matchTrs):
            # print(tr)
            matchTds = re.findall(r'<td>(.*?)</td>', tr)
            if len(matchTds) == 3:
                print("%d,%s,%s" % (index, matchTds[1], matchTds[2]), file=fOut)
                sIndex = str(index)
                if (sIndex in status) and (matchTds[1] != str(status[sIndex])):
                    print("!issue with %d %s != %s" % (index, matchTds[1], status[sIndex]))

    subprocess.check_call(os.path.join(this_dir, 'gap.py'), shell=True)
