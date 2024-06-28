import cachalot

import sys
import unittest
import json
import time
import datetime
import string

import random
import threading

server = "http://localhost:5555"
token = "cb57466281a8f398aa63416b8b499978"
project = "NUM"

class Testing(unittest.TestCase):
    def __test(self):
        for i in range(500):
            rnd = random.randrange(0, 1696072383)
            data_insert = {}
            records = []
            data_insert["records"] = records
            
            for j in range(random.randrange(1, 10)):
                record = {}
                record["user.id"] = "user{0}".format(i)
                record["level"] = random.randrange(1, 5)
                record["service"] = "test"
                record["message"] = ''.join(random.choices(string.ascii_lowercase, k=random.randrange(1, 50))) + "END1234567890_" + str(j)
                record["file"] = random.choice(["test.py", "main.py", "print.py", "cachalot.py", "gen.py"])
                record["line"] = random.randint(1, 1000)
                record["timestamp"] = int(datetime.datetime.now().timestamp())
                record["live"] = 2383
                record["build.environment"] = random.choice(["dev", "prod", "staging"])
                record["build.release"] = True
                record["build.version"] = random.choice(["1.0.0", "1.1.0-dev", "1.1.8-nightly"])
                record["build.number"] = 12345
                
                record["device.model"] = random.choice(["iPhone 11", "iPhone 11 Pro", "iPhone 11 Pro Max"])
                
                record["os.family"] = "TestOS"
                record["os.version"] = random.choice(["12.0", "11.0", "10.4", "10.0"])
                
                record["attributes"] = {}
                record["attributes"]["cik_value"] = "0697e656ca1824f5"
                record["attributes"]["cik_timestamp"] = "1696072383"
                
                record["tags"] = random.choices(["game", "food", "ai", "net", "service", "test"], k=random.randrange(0, 4))
                
                records.append(record)
            
            print("insert token: {0} data: {1}".format(token, data_insert))
            jresult_insert = cachalot.post(server, "{0}/{1}/insert".format(token, project), **data_insert)
            print("response: ", json.dumps(jresult_insert))
            
            self.assertIsNotNone(jresult_insert)
        pass
        
        data_select = {}
        data_select["time.offset"] = 0
        data_select["time.limit"] = 500
        data_select["count.limit"] = 500
    
        print("select token: {0} data: {1}".format(token, data_select))
        jresult_select = cachalot.post(server, "{0}/{1}/select".format(token, project), **data_select)
        print("response: ", json.dumps(jresult_select))

        self.assertIsNotNone(jresult_select)
        pass

    def test_00_log(self):
        self.__test()
        pass
    
    def test_01_thread(self):
        threads = []
        for index in range(32):
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
