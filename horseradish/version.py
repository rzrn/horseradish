# this is a wrapper around horseradish/_version.py, provided by setuptools-scm
# pyright: reportMissingImports=false
from horseradish._version import (__version__ as VERSION,
                                  __version_tuple__ as VERSION_TUPLE)

__version__ = VERSION
__version_info__ = VERSION_TUPLE
