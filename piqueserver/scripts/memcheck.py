"""
Runs the garbage collector at a given interval and displays any uncollected
garbage found.

.. codeauthor:: mat^2
"""

import asyncio
import gc

INTERVAL = 60 * 10
VERBOSE = False

def apply_script(protocol, connection, config):
    async def garbage_collector_loop():
        while True:
            await asyncio.sleep(INTERVAL)

            ret = gc.collect()
            if VERBOSE:
                print('gc.collect() ->', ret)
            if not gc.garbage:
                pass
            else:
                print('Memory leak detected!')
                print('Contents of gc.garbage:', gc.garbage)

    class GarbageCollectorProtocol(protocol):
        async def on_event_loop_start(self):
            await protocol.on_event_loop_start(self)
            self.create_task(garbage_collector_loop())


    return GarbageCollectorProtocol, connection
