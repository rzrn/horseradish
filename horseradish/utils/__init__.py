from ._timeparse import timeparse
from ._async import EndCall

import os

def ensure_dir_exists(filename : str) -> None:
    d = os.path.dirname(filename)
    os.makedirs(d, exist_ok = True)