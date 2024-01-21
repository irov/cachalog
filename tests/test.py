import cachalot

import sys
import unittest
import json
import time
import string

import random
import threading

server = "http://localhost:5555"

class Testing(unittest.TestCase):
    def __test(self):
        token = "cb57466281a8f398aa63416b8b499978"
    
        for i in range(500):
            rnd = random.randrange(0, 1696072383)
            data_insert = {}
            records = []
            data_insert["records"] = records
            
            for j in range(random.randrange(1, 10)):
                record = {}
                record["user.id"] = "user{0}".format(i)
                record["level"] = 3
                record["service"] = "test"
                record["message"] = ''.join(random.choices(string.ascii_lowercase, k=random.randrange(1, 50))) + "END1234567890_" + str(j)
                record["file"] = "test.cpp"
                record["line"] = 378
                record["timestamp"] = int(time.time())
                record["live"] = 2383
                record["build.environment"] = "dev"
                record["build.release"] = True
                record["build.version"] = "1.0.0"
                record["build.number"] = 12345
                
                record["device.model"] = "12345"
                
                record["os.family"] = "Android"
                record["os.version"] = "12"
                
                record["attributes"] = {}
                record["attributes"]["cik_value"] = "0697e656ca1824f5"
                record["attributes"]["cik_timestamp"] = "1696072383"
                
                record["tags"] = []
                record["tags"] += ["game"]
                record["tags"] += ["food"]
                
                records.append(record)
            
            print("insert token: {0} data: {1}".format(token, data_insert))
            jresult_insert = cachalot.post(server, "{0}/insert".format(token), **data_insert)
            print("response: ", jresult_insert)
            
            self.assertIsNotNone(jresult_insert)
        pass
        
        data_select = {}
        data_select["time.offset"] = 0
        data_select["time.limit"] = 500
        data_select["count.limit"] = 500
    
        print("select token: {0} data: {1}".format(token, data_select))
        jresult_select = cachalot.post(server, "{0}/select".format(token), **data_select)
        print("response: ", json.dumps(jresult_select))

        self.assertIsNotNone(jresult_select)
        pass

    def test_00_log(self):
        self.__test()
        pass
    
    def test_01_thread(self):
        threads = []
        for index in range(64):
            x = threading.Thread(target=self.__test, args=())
            threads.append(x)
            x.start()
            pass

        for thread in threads:
            thread.join()
            pass
        pass
    pass

unittest.main()