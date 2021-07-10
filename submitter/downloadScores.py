#!/usr/bin/env python3

import requests
import re

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
