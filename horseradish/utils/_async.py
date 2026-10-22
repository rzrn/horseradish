# Copyright © 2011–2012 Mathias Kaerlev
# Copyright © 2019 Godwhoa
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
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import asyncio
from asyncio import Future
from typing import Awaitable, Optional, Callable


# TODO: this looks ugly and probably is not really needed
class EndCall:
    """a call that can be rescheduled while in the future"""
    def __init__(self, protocol, delay: int, func: Callable, *arg, **kw) -> None:
        self.protocol = protocol
        protocol.end_calls.append(self)
        self.delay = delay
        self.func = func
        self.arg = arg
        self.kw = kw
        self.call = None  # type: Optional[Deferred]
        self._active = True

    def set(self, value: Optional[float]) -> None:
        if value is None:
            if call := self.call:
                self.call = None
                call.cancel()
        else:
            value = value - self.delay

            if value <= 0.0:
                self.cancel()
            else:
                if call := self.call:
                    call.cancel()

                self.call = asyncio.get_running_loop().call_later(value, self.fire)

    def fire(self):
        self.call = None
        self.cancel() # TODO: do we need this?
        self.func(*self.arg, **self.kw)

    def cancel(self) -> None:
        if self._active:
            self.set(None)
            self.protocol.end_calls.remove(self)
            self._active = False

    def active(self) -> bool:
        return self._active and self.call is not None and self.call.cancelled() is False
