# Copyright © 2011–2012 Mathias Kaerlev
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

"""
The map generator is responsible for generating the map bytes that get sent
to the client on connect
"""


class ProgressiveMapGenerator:
    """
    Progressively generates the stream of bytes sent to the client for map
    downloads.

    It supports two modes. In the default parent=False mode, reading is normal.

    In the `parent=True` mode a child generator is created with `get_child` to
    actually read the data. This is presumably done so that the work of map
    generation is not duplicated for each client if several connect at the same
    time.
    """
    data = b''
    done = False

    # parent attributes
    all_data = b''
    pos = 0

    def __init__(self, map_, parent=False):
        # parent=True enables saving all data sent instead of just
        # deleting it afterwards.
        self.parent = parent
        self.mapenc = map_.get_encoder()

    def get_size(self):
        """get the map size, for display of the loading bar on the client"""
        # This is currently just an estimate, since due to compression
        # magic, we don't actually know what size the file will be when sent
        # over the wire
        return 1.5 * 1024 * 1024  # 2MB

    def read(self, size):
        """read size bytes from the map generator"""
        data = self.data
        mapenc = self.mapenc

        if len(data) < size and mapenc is not None:
            while True:
                if mapenc.is_eof():
                    self.mapenc = None
                    break

                data += self.mapenc.encode()
                if len(data) >= size:
                    break

        if self.parent:
            # save the data in case we are a parent
            self.all_data += data
            self.pos += len(data)
        else:
            self.data = data[size:]
            return data[:size]

    def get_child(self):
        """return a new child generator"""
        if self.parent:
            return MapGeneratorChild(self)
        else:
            raise NotImplementedError(
                "get_child is not implemented for non-parent generators")

    def data_left(self):
        """return True if any data is left"""
        return bool(self.data) or self.mapenc is not None


class MapGeneratorChild:
    pos = 0

    def __init__(self, generator):
        self.parent = generator

    def get_size(self):
        """get the size of the parent map generator"""
        return self.parent.get_size()

    def read(self, size):
        """read size bytes from the parent map generator, if possible"""
        pos = self.pos
        if pos + size > self.parent.pos:
            self.parent.read(size)
        data = self.parent.all_data[pos:pos + size]
        self.pos += len(data)
        return data

    def data_left(self):
        """return True if any data is left"""
        return self.parent.data_left() or self.pos < self.parent.pos
