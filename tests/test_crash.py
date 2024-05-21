import os
import unittest
import threading
# import multiprocessing

from random import random
from random import randrange
from time import time

if __name__.startswith("tests"):
    from . import cachalot
    from .conftest import SERVER, TOKEN, PROJECT
    from .record_fabric import LogRecordFabric
else:
    import cachalot
    from conftest import SERVER, TOKEN, PROJECT
    from record_fabric import LogRecordFabric


class TestCrashDuringLargeDataRequest(unittest.TestCase):
    def new_mock_datablock(self):
        data = {}
        LogFabric = LogRecordFabric()
        data["records"] = [LogFabric.create(i) for i in range(randrange(1, 10))]
        return data

    def send_mock_data(self, get_stop_flag):
        data = self.new_mock_datablock()
        for i in range(500):
            jresult = cachalot.post(
                SERVER, "{0}/{1}/insert".format(TOKEN, PROJECT), **data
            )
            assert jresult is not None
            if get_stop_flag():
                break

    def test_sending_data(self):
        threads = []
        is_stop = False
        # pool = multiprocessing.Pool(processes=8)
        # pool.map(self.send_mock_data, [lambda: is_stop])
        for _ in range(8):
            th = threading.Thread(target=self.send_mock_data, args=([lambda: is_stop]))
            threads.append(th)
            th.start()

        try:
            # pool.join()
            for thread in threads:
                thread.join()
        except KeyboardInterrupt:
            print("KeyboardInterrupt, stopping...")
            is_stop = True

    def test_crash(self):
        select_data = {"count.limit": 300}
        cachalot.post(SERVER, "{0}/{1}/select".format(TOKEN, PROJECT), **select_data)


if __name__ == "__main__":
    unittest.main()
