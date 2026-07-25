# Copyright © 2026 rzrn

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import string
import inspect
import logging

formatter = string.Formatter()

class FormatEvent:
    def __init__(self, fmt, w, kw):
        self.fmt    = str(fmt)
        self.args   = w
        self.kwargs = kw

    def __str__(self):
        return formatter.vformat(self.fmt, self.args, self.kwargs)

class FormatLogger(logging.Logger):
    def _log(self, level, msg, args, exc_info = None, extra = None, stack_info = False, stacklevel = 1, **kwargs):
        if kwargs:
            msg = FormatEvent(msg, args, kwargs)
            args = ()

        super()._log(
            level, msg, args,
            exc_info = exc_info,
            extra = extra,
            stack_info = stack_info,
            stacklevel = stacklevel + 1
        )

logging.setLoggerClass(FormatLogger)

def getLogger(name = None):
    if name is None:
        name = inspect.currentframe().f_back.f_globals['__name__']

    return logging.getLogger(name)
