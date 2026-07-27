# Copyright (c) Mathias Kaerlev 2012.

# This file is part of pyspades.

# pyspades is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# pyspades is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with pyspades.  If not, see <http://www.gnu.org/licenses/>.

import asyncio
import functools
from weakref import WeakSet

async def looping_call(delay, func, w, kw):
    while True:
        await asyncio.sleep(delay)
        func(*w, **kw)

class Scheduler:
    def __init__(self, protocol):
        self.protocol = protocol
        self.calls = WeakSet()
        self.loops = WeakSet()

    def call_later(self, delay, func, *w, **kw):
        part = functools.partial(func, *w, **kw)
        call = asyncio.get_running_loop().call_later(delay, part)
        self.calls.add(call)
        return call

    def call_end(self, *w, **kw):
        call = self.protocol.call_end(*w, **kw)
        self.calls.add(call)
        return call

    def loop_call(self, delay, func, *w, **kw):
        loop = self.protocol.create_task(looping_call(delay, func, w, kw))
        self.loops.add(loop)
        return loop

    def reset(self):
        for call in self.calls:
            call.cancel()

        for loop in self.loops:
            loop.cancel()

        self.calls = WeakSet()
        self.loops = WeakSet()
