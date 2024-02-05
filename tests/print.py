import cachalot

import json

server = "http://localhost:5555"
token = "cb57466281a8f398aa63416b8b499978"
project = "NUM"

data_select = {}
data_select["time.offset"] = 0
data_select["time.limit"] = 500
data_select["count.limit"] = 500

jresult_select = cachalot.post(server, "{0}/{1}/select".format(token, project), **data_select)

for record in jresult_select["records"]:
    print(record["message"])