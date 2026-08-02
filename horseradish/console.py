# Copyright (c) Mathias Kaerlev 2011-2012.

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

from threading import Thread
from typing import List
import traceback
import sys

import asyncio

from pyspades.types import AttributeSet
from horseradish import commands


class StdinDaemonThread(Thread):
    def __init__(self, loop = None):
        super().__init__(daemon = True)
        self.queue = asyncio.Queue()

        if loop is None:
            self.loop = asyncio.get_running_loop()
        else:
            self.loop = loop

    def run(self):
        while True:
            recv = input()
            self.loop.call_soon_threadsafe(self.queue.put_nowait, recv)

    async def input(self):
        return await self.queue.get()


class ConsoleInput:
    name = 'Console'
    admin = True
    delimiter = b'\n'

    def __init__(self, protocol):
        self.protocol = protocol
        self.user_types = AttributeSet(['admin', 'console'])
        self.rights = AttributeSet()
        for user_type in self.user_types:
            self.rights.update(commands.get_rights(user_type))

    # methods used to emulate the behaviour of regular Connection objects to
    # prevent errors when command writers didn't test that their scripts would
    # work when run on the console
    def send_chat(self, value: str, _):
        print(value)

    def send_lines(self, lines: List[str], type: str = None):
        print("\n".join(lines))


async def create_console(protocol):
    console = ConsoleInput(protocol)

    # The reason for this insanity is that this thread will be automatically
    # closed without waiting for `input()` as it is a daemon thread.
    thrdin = StdinDaemonThread()
    thrdin.start()

    while True:
        # This appears to be the most portable way to do it.
        # Since we are not passing gigabytes through stdin anyway,
        # there is no reason to add unnecessary complexity.
        recv = await thrdin.input()

        if not recv:
            continue

        try:
            retval = commands.handle_input(console, recv)
        except Exception:
            traceback.print_exc()
        else:
            if retval is not None:
                print(retval)
