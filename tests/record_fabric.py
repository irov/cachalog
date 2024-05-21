import random
import string
import datetime


TEMPLATES = {
    "files": ["test.py", "main.py", "print.py", "cachalot.py", "gen.py"],
    "envs": ["dev", "prod", "staging"],
    "versions": ["1.0.0", "1.1.0-dev", "1.1.8-nightly"],
    "models": ["iPhone 11", "iPhone 11 Pro", "iPhone 11 Pro Max"],
    "os": ["12.0", "11.0", "10.4", "10.0"],
    "tags": ["game", "food", "ai", "net", "service", "test"],
}


class LogRecordFabric:
    def __init__(self) -> None:
        pass

    def _get_msg(self):
        end_suffix = "END1234567890_"
        alphabet = string.ascii_lowercase
        msg = "".join(random.choices(alphabet, k=random.randrange(1, 50)))
        return msg + end_suffix

    def create(self, *args, **kwargs):
        record = {}
        record["user.id"] = "user_{0}".format(random.randrange(0, 10))
        record["level"] = random.randrange(1, 5)
        record["service"] = "test"
        record["message"] = self._get_msg()
        record["file"] = random.choice(TEMPLATES["files"])
        record["line"] = random.randint(1, 1000)
        record["timestamp"] = int(datetime.datetime.now().timestamp())
        record["live"] = 2383
        record["build.environment"] = random.choice(TEMPLATES["envs"])
        record["build.release"] = True
        record["build.version"] = random.choice(TEMPLATES["versions"])
        record["build.number"] = 12345

        record["device.model"] = random.choice(TEMPLATES["models"])

        record["os.family"] = "TestOS"
        record["os.version"] = random.choice(TEMPLATES["os"])

        record["attributes"] = {}
        record["attributes"]["cik_value"] = "0697e656ca1824f5"
        record["attributes"]["cik_timestamp"] = "1696072383"

        record["tags"] = random.choices(TEMPLATES["tags"], k=random.randrange(0, 4))

        return record
