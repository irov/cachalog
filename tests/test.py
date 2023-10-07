import cachalog

import sys
import unittest
import json

class Testing(unittest.TestCase):
    def test_00_log(self):
        token = "cb57466281a8f398aa63416b8b499978"
    
        data = {}
        data["service"] = "test"
        data["id"] = "user12345678"
        data["message"] = "Test log message!"
        data["category"] = "common"
        data["level"] = 3
        data["timestamp"] = 1696072383
        data["live"] = 2383
        data["build.environment"] = "dev"
        data["build.release"] = True
        data["build.version"] = "1.0.0"
        data["build.number"] = 12345
        data["device.model"] = "12345"
        data["os.family"] = "Android"
        data["os.version"] = "12"
        
        data["attributes"] = {}
        data["attributes"]["cik_value"] = "0697e656ca1824f5"
        data["attributes"]["cik_timestamp"] = "1696072383"
        
        print("log token: {0} data: {1}".format(token, data))
        jresult = cachalog.post("http://localhost:5555/{0}/log".format(token), **data)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        pass
    pass

unittest.main()