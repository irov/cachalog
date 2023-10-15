import cachalog

import sys
import unittest
import json
import time
import string

import random

class Testing(unittest.TestCase):
    def test_00_log(self):
        token = "cb57466281a8f398aa63416b8b499978"
    
        for i in range(50):
            rnd = random.randrange(0, 1696072383)
            data_insert = {}
            data_insert["service"] = "test"
            data_insert["user.id"] = "user{0}".format(rnd)
            data_insert["message"] = ''.join(random.choices(string.ascii_lowercase, k=random.randrange(1, 4096)))
            data_insert["category"] = "common"
            data_insert["level"] = 3
            data_insert["timestamp"] = int(time.time()) 
            data_insert["live"] = 2383
            data_insert["build.environment"] = "dev"
            data_insert["build.release"] = True
            data_insert["build.version"] = "1.0.0"
            data_insert["build.number"] = 12345
            data_insert["device.model"] = "12345"
            data_insert["os.family"] = "Android"
            data_insert["os.version"] = "12"
            
            data_insert["attributes"] = {}
            data_insert["attributes"]["cik_value"] = "0697e656ca1824f5"
            data_insert["attributes"]["cik_timestamp"] = "1696072383"
            
            print("insert token: {0} data: {1}".format(token, data_insert))
            jresult_insert = cachalog.post("http://localhost:5555/{0}/insert".format(token), **data_insert)
            print("response: ", jresult_insert)
            
            self.assertIsNotNone(jresult_insert)
            
            data_select = {}
            data_select["time.offset"] = 0
            data_select["time.limit"] = 100
            data_select["count.limit"] = 10
        
            print("select token: {0} data: {1}".format(token, data_select))
            jresult_select = cachalog.post("http://localhost:5555/{0}/select".format(token), **data_select)
            print("response: ", jresult_select)

            self.assertIsNotNone(jresult_select)
        pass
    pass

unittest.main()