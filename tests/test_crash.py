import os

from random import random
from random import randrange
from time import time
import unittest
import threading

ENV = "pytest" if __name__.startswith("tests") else "python"

if ENV == "pytest":
    from . import cachalot
    from .conftest import SERVER, TOKEN, PROJECT
    from .record_fabric import LogRecordFabric
else:
    import cachalot
    from conftest import SERVER, TOKEN, PROJECT
    from record_fabric import LogRecordFabric


LogFabric = LogRecordFabric()

class TestCrashDuringLargeDataRequest(unittest.TestCase):
    def new_mock_datablock(self):
        data = {}
        data["records"] = [LogFabric.create(i) for i in range(randrange(1, 10))]
        return data

    def send_mock_data(self, get_stop_flag):
        data = self.new_mock_datablock()
        for i in range(10):
            jresult = cachalot.post(
                SERVER, "{0}/{1}/insert".format(TOKEN, PROJECT), **data
            )
            assert jresult is not None
            if get_stop_flag():
                break

    def test_sending_data(self):
        print("ADDRESS: {}/{}/{}/insert".format(SERVER, TOKEN, PROJECT))
        start_time = time()
        threads = []
        is_stop = False
        for index in range(4):
            th = threading.Thread(target=self.send_mock_data, args=([lambda: is_stop]))
            threads.append(th)
            th.start()

        try:
            for thread in threads:
                thread.join()
        except KeyboardInterrupt:
            print("KeyboardInterrupt, stopping...")
            is_stop = True

        print("SENDING_DATA_TIME: {0}".format(time() - start_time))

    def test_crash(self):
        print("ADDRESS: {}/{}/{}/select".format(SERVER, TOKEN, PROJECT))
        start_time = time()
        data = {"count.limit": 3000}
        cachalot.post(SERVER, "{0}/{1}/select".format(TOKEN, PROJECT), **data)
        print("SENDING_DATA_TIME: {0}".format(time() - start_time))


if __name__ == "__main__":
    unittest.main()
