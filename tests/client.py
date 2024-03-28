"""
Signature:
python mock_data.py --server="https://hummingfab.com/api/grid/" --token=12345678901234567890123456789012 --project="Project Name"

Params (Can be set inside script):
    * server [OPTIONAL]: string as url - server url
    * token [OPTIONAL]: string[32] - token for server
    * project [OPTIONAL]: string - project name as on site
"""

import sys
import platform

DEFAULT_SERVER = "https://hummingfab.com/api/grid/"
DEFAULT_TOKEN = "cb57466281a8f398aa63416b8b499978"
DEFAULT_PROJECT = "TM-DEV"

def get_os_name():
    os_name = platform.system().lower()
    if os_name.startswith('cygwin') or os_name.startswith('mingw') or os_name.startswith('msys'):
        return 'windows'
    if os_name == 'linux':
        return 'linux'

    return os_name

is_py2 = (sys.version_info[0] == 2)
is_py3 = (sys.version_info[0] == 3)
is_win = get_os_name() == 'windows'

class BaseError(Exception):
    pass


class ValidationError(BaseError):
    pass


class InputParser:
    def __init__(self, server, token, project) -> None:
        self.server = server
        self.token = token
        self.project = project

    @classmethod
    def from_sys_args(cls):
        if "--help" in sys.argv or "-h" in sys.argv or len(sys.argv) < 2:
            print(__doc__)
            sys.exit(0)

        server = DEFAULT_SERVER
        token = DEFAULT_TOKEN
        project = DEFAULT_PROJECT

        for element in sys.argv[1:]:
            if element.startswith("--server"):
                server = element.split("=")[1]
            elif element.startswith("--token"):
                token = element.split("=")[1]
            elif element.startswith("--project"):
                project = element.split("=")[1]
            else:
                print("[WARN] Ignoring unknown param: {0}".format(element))

        return cls(server, token, project)


class RuntimeEnv:
    server: str
    token: str
    project: str

    def __init__(self, input_parser: InputParser) -> None:
        self.server = self._validate_server_url(input_parser.server)
        self.token = self._validate_token(input_parser.token)
        self.project = self._validate_project(input_parser.project)
        print("Init config: {0} {1} {2}".format(self.server, self.token, self.project))

    def _validate_server_url(self, server_url):
        if server_url is None:
            raise ValidationError("server_url is not defined, more in help msg")
        if server_url[-1] != "/":
            server_url += "/"
        return server_url

    def _validate_token(self, token):
        if token is None:
            raise ValidationError("token is not defined, more in help msg")
        if len(token) != 32:
            raise ValidationError("token size is not valid, need 32")
        return token

    def _validate_project(self, project):
        if project is None:
            raise ValidationError("project is not defined, more in help msg")
        return project


if __name__ == "__main__":
    ip = InputParser.from_sys_args()
    re = RuntimeEnv(ip)

