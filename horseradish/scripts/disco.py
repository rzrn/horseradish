"""
Ever wanted a disco in Ace of Spades?

Commands
^^^^^^^^

* ``/disco`` toggles disco

.. codeauthor:: mat^2
"""

from itertools import cycle
import asyncio

from horseradish.commands import command, admin

DISCO_ON_GAME_END = True
# Time is in seconds
DISCO_ON_GAME_END_DURATION = 10.0


@command('disco', admin_only=True)
def toggle_disco(connection):
    connection.protocol.toggle_disco_loop()


DISCO_COLORS = [
    (235, 64, 0),
    (128, 232, 121),
    (220, 223, 12),
    (43, 72, 228),
    (216, 94, 231),
    (255, 255, 255)
]


def apply_script(protocol, connection, config):
    class DiscoProtocol(protocol):
        old_fog_color = None
        disco_loop = None

        def __init__(self, *w, **kw):
            protocol.__init__(self, *w, **kw)

            self.disco_colors_iter = cycle(DISCO_COLORS)

        async def disco_update_color(self, interval = 0.3):
            while True:
                self.set_fog_color(next(self.disco_colors_iter))

                await asyncio.sleep(interval)

        def on_game_end(self):
            if self.disco_loop is None and DISCO_ON_GAME_END:
                self.start_disco_loop()

                asyncio.get_running_loop().call_later(
                    DISCO_ON_GAME_END_DURATION, self.stop_disco_loop
                )

            return protocol.on_game_end(self)

        def start_disco_loop(self):
            if self.disco_loop is None:
                self.old_fog_color = self.fog_color

                self.disco_loop = self.create_task(
                    self.disco_update_color()
                )

        def stop_disco_loop(self):
            if defer := self.disco_loop:
                self.disco_loop = None
                defer.cancel()

                if fog := self.old_fog_color:
                    self.set_fog_color(fog)

        def toggle_disco_loop(self):
            if self.disco_loop is None:
                self.broadcast_chat('DISCO PARTY MODE ENABLED!')
                self.start_disco_loop()
            else:
                self.broadcast_chat('The party has been stopped.')
                self.stop_disco_loop()


    return DiscoProtocol, connection
