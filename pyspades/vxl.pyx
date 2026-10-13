# Copyright (c) Mathias Kaerlev 2011-2012.

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

from libc.stdlib cimport calloc, malloc, free
from cython cimport boundscheck

import random
import time

import zlib

cdef extern from "Python.h":
    object PyBytes_FromStringAndSize(char *, Py_ssize_t)
    char * PyBytes_AsString(object)

cdef struct MapColXY:
    int x
    int y

cdef inline int clamp(int x1, int x2, int x):
    return min(x2, max(x1, x))

cdef enum:
    MapCompressionLevel = 5
    MapEncoderNcols = 8192

cdef class MapEncoder:
    cdef MapBuf mapbuf
    cdef long xy

    # `cols * 65 * 4` is a magic number taken from ‘lsd’
    # and it’d be better to hold it somewhere in ‘libpvx2’.
    cdef unsigned char buff[MapEncoderNcols * 65 * 4]
    cdef object stream

    cdef object vxl

    def __init__(self, VXLData vxl):
        # We store it to make sure that `mapbuf.colorbuf` is valid
        self.vxl = vxl

        map_copy(&self.mapbuf, vxl.mapbuf)

        # We use the fact that `map_destroy` does not remove colors, so the worst
        # thing that could happen is a wrong color sent to the player, which is
        # not a big deal compared to the much lower memory consumption here.
        self.mapbuf.colorbuf = vxl.mapbuf.colorbuf

        self.stream = zlib.compressobj(MapCompressionLevel)
        self.xy = 0

    def encode(self):
        if self.xy < 0:
            raise EOFError

        cdef long xy = min(self.xy + MapEncoderNcols, MAP_XY)
        cdef long cols = xy - self.xy

        cdef size_t size

        if cols > 0:
            size = vxl_write(self.buff, &self.mapbuf, self.xy, cols)
            self.xy = xy

            return self.stream.compress(self.buff[:size])
        else:
            self.vxl = None
            self.xy = -1

            return self.stream.flush()

    def is_eof(self):
        return self.xy < 0

cdef class VXLData:
    def __cinit__(self, *w, **kw):
        with nogil:
            self.mapbuf   = <MapBuf *> calloc(1, sizeof(MapBuf))
            self.cullbuf  = <CullBuf *> calloc(1, sizeof(CullBuf))
            self.colorbuf = <unsigned char *> malloc(3 * MAP_XYZ)

            self.mapbuf.colorbuf = self.colorbuf

    def __dealloc__(self):
        free(self.mapbuf)
        free(self.cullbuf)
        free(self.colorbuf)

    def __init__(self, fp = None):
        if isinstance(fp, VXLData):
            with nogil:
                map_copy(self.mapbuf, (<VXLData> fp).mapbuf)
        elif fp is not None:
            self.load_bytes(fp.read())
        else:
            with nogil:
                map_clear(self.mapbuf)

    @boundscheck(False) # Just to silence Cython warning.
    cdef load_bytes(self, bytes mapdata):
        cdef const unsigned char[:] data = mapdata
        cdef size_t size = len(mapdata)

        with nogil:
            vxl_read(self.mapbuf, &data[0], size)

    def load_vxl(self, c_data = None):
        if c_data is None:
            with nogil:
                map_clear(self.mapbuf)
        else:
            self.load_bytes(c_data)

    def copy(self):
        return VXLData(self)

    def get_point(self, int x, int y, int z):
        color = self.get_color(x, y, z)
        solid = color is not None
        return solid, color

    def set_point(self, int x, int y, int z, tuple color):
        if is_valid_position(x, y, z) is False:
            return

        map_create(self.mapbuf, x, y, z)

        r, g, b = color
        cdef unsigned char[3] colorbuf = [b, g, r]
        map_set_color(self.mapbuf, x, y, z, colorbuf, 1)

    # TODO: consider making this function raise error on invalid position
    cpdef get_solid(self, int x, int y, int z):
        if not is_valid_position(x, y, z):
            return None

        return map_is_solid(self.mapbuf, x, y, z)

    cpdef get_color(self, int x, int y, int z):
        if not self.get_solid(x, y, z):
            return None

        cdef unsigned char * color = map_get_color(self.mapbuf, x, y, z)

        return color[2], color[1], color[0]

    def is_valid_position(self, int x, int y, int z):
        """return if the value is a valid position within the bounds of the
        map"""
        return is_valid_position(x, y, z)

    cpdef int get_z(self, int x, int y, int start = 0):
        '''
        Returns the first z coordinate that is solid beginning from `start` and
        moving down.  Useful for getting the coordinate for where something
        should be after being dropped.
        '''

        if not is_valid_position(x, y, 63):
            return 63

        return map_get_floor(self.mapbuf, x, y, clamp(0, 63, start))

    cpdef int get_height(self, int x, int y):
        if not is_valid_position(x, y, 63):
            return 64

        cdef int start = 63

        for z in range(start, -1, -1):
            if not map_is_solid(self.mapbuf, x, y, z):
                return z + 1

        return 0

    cpdef tuple get_safe_coords(self, int x, int y, int z):
        '''
        given (x, y, z) coords, return the closest set of coords on the map
        that is within the bounds of the map
        '''

        return clamp(0, 511, x), clamp(0, 511, y), clamp(0, 63, z)

    cpdef tuple get_random_point(self, int x1, int y1, int x2, int y2):
        x1, x2 = clamp(0, 512, x1), clamp(0, 512, x2)
        y1, y2 = clamp(0, 512, y1), clamp(0, 512, y2)

        x1, x2 = min(x1, x2), max(x1, x2)
        y1, y2 = min(y1, y2), max(y1, y2)

        cdef size_t sizeX = x2 - x1
        cdef size_t sizeY = y2 - y1

        cdef MapColXY * buff = <MapColXY *> malloc(sizeX * sizeY * sizeof(MapColXY))

        cdef size_t size = 0

        for x in range(x1, x2):
            for y in range(y1, y2):
                if map_is_solid(self.mapbuf, x, y, 62):
                    buff[size].x = x
                    buff[size].y = y

                    size += 1

        cdef MapColXY choice

        if size > 0:
            choice = buff[random.randint(0, size - 1)]
        else:
            choice.x = random.randint(x1, x2 - 1)
            choice.y = random.randint(y1, y2 - 1)

        free(buff)

        return choice.x, choice.y

    def count_land(self, int x1, int y1, int x2, int y2):
        x1, x2 = clamp(0, 512, x1), clamp(0, 512, x2)
        y1, y2 = clamp(0, 512, y1), clamp(0, 512, y2)

        x1, x2 = min(x1, x2), max(x1, x2)
        y1, y2 = min(y1, y2), max(y1, y2)

        cdef int land = 0

        for x in range(x1, x2):
            for y in range(y1, y2):
                if map_is_solid(self.mapbuf, x, y, 62):
                    land += 1

        return land

    def destroy_point(self, int x, int y, int z):
        if not self.get_solid(x, y, z) or z >= 62:
            return 0

        map_destroy(self.mapbuf, x, y, z)

        count = 1
        start = time.monotonic()
        for node_x, node_y, node_z in self.get_neighbors(x, y, z):
            if node_z < 62:
                count += self.cull(node_x, node_y, node_z)
        taken = time.monotonic() - start
        if taken > 0.1:
            print('destroying block at', x, y, z, 'took:', taken)

        return count

    def remove_point(self, int x, int y, int z):
        if is_valid_position(x, y, z):
            map_destroy(self.mapbuf, x, y, z)

    cpdef bint has_neighbors(self, int x, int y, int z):
        return (
            self.get_solid(x + 1, y, z) or
            self.get_solid(x - 1, y, z) or
            self.get_solid(x, y + 1, z) or
            self.get_solid(x, y - 1, z) or
            self.get_solid(x, y, z + 1) or
            self.get_solid(x, y, z - 1)
        )

    cpdef bint is_surface(self, int x, int y, int z):
        return (
            not self.get_solid(x, y, z - 1) or
            not self.get_solid(x, y, z + 1) or
            not self.get_solid(x + 1, y, z) or
            not self.get_solid(x - 1, y, z) or
            not self.get_solid(x, y + 1, z) or
            not self.get_solid(x, y - 1, z)
        )

    cpdef list get_neighbors(self, int x, int y, int z):
        cdef list neighbors = []
        for (node_x, node_y, node_z) in (
            (x, y, z - 1),
            (x, y - 1, z),
            (x, y + 1, z),
            (x - 1, y, z),
            (x + 1, y, z),
            (x, y, z + 1)):
            if self.get_solid(node_x, node_y, node_z):
                neighbors.append((node_x, node_y, node_z))
        return neighbors

    cpdef int cull(self, int x, int y, int z):
        cdef int removed

        with nogil:
            removed = map_cull(self.mapbuf, self.cullbuf, x, y, z)
            map_cull_end(self.cullbuf)

        return removed

    cpdef bint build_point(self, int x, int y, int z, tuple color):
        if not is_valid_position(x, y, z):
            return False

        if not self.has_neighbors(x, y, z) or z >= 62:
            return False

        map_create(self.mapbuf, x, y, z)

        r, g, b = color
        cdef unsigned char[3] colorbuf = [b, g, r]
        map_set_color(self.mapbuf, x, y, z, colorbuf, 1)

        return True

    cpdef bint set_column_fast(self, int x, int y, int z_start,
        int z_end, int z_color_end, int color):
        """Set a column's solidity, but only color a limited amount from
            the top."""
        if (not is_valid_position(x, y, z_start) or
            not is_valid_position(x, y, z_end) or
            z_end < z_start):
            return False

        cdef long xy = map_to_xy(x, y)

        map_create_range_xy(self.mapbuf, xy, z_start, z_end - z_start + 1)

        if not is_valid_position(x, y, z_color_end) or z_color_end < z_start:
            return False

        # TODO: make this function accept `tuple` as well
        cdef unsigned char[3] colorbuf = [
            (color >> 0)  & 0xFF,
            (color >> 8)  & 0xFF,
            (color >> 16) & 0xFF
        ]

        for z in range(z_start, z_color_end + 1):
            map_set_color_xy(self.mapbuf, xy, z, colorbuf, 1)

        return True

    def get_overview(self, int z = -1, bint rgba = False):
        # TODO: what is the purpose of this function at all?

        cdef unsigned int i, r, g, b, a
        cdef unsigned char * color

        cdef object retval = PyBytes_FromStringAndSize(NULL, sizeof(int[512][512]))
        cdef unsigned int * data = <unsigned int *> PyBytes_AsString(retval)

        i = 0
        cdef int current_z
        if z == -1:
            a = 255
        else:
            current_z = z
        for y in range(512):
            for x in range(512):
                if z == -1:
                    current_z = self.get_z(x, y)
                else:
                    if map_is_solid(self.mapbuf, x, y, z):
                        a = 255
                    else:
                        a = 0

                color = map_get_color(self.mapbuf, x, y, current_z)

                b = color[0]
                g = color[1]
                r = color[2]

                if rgba:
                    data[i] = r | (g << 8) | (b << 16) | (a << 24)
                else:
                    data[i] = b | (g << 8) | (r << 16) | (a << 24)

                i += 1

        return retval

    def set_overview(self, data_str, int z):
        cdef unsigned int * data
        cdef unsigned char colorbuf[3]
        cdef unsigned int r, g, b, a, color, i

        data = <unsigned int*>(<char*>data_str)

        i = 0

        for y in range(512):
            for x in range(512):
                color = data[i]
                a = (color & <unsigned int> 0xFF000000) >> 24
                if a != 255:
                    map_destroy(self.mapbuf, x, y, z)
                else:
                    map_create(self.mapbuf, x, y, z)

                    colorbuf[0] = (color >> 0)  & 0xFF
                    colorbuf[1] = (color >> 8)  & 0xFF
                    colorbuf[2] = (color >> 16) & 0xFF

                    map_set_color(self.mapbuf, x, y, z, colorbuf, 1)

                i += 1

    def generate(self):
        t1 = time.monotonic()

        cdef unsigned char * buff = <unsigned char *> malloc(10 * 1024 * 1024)
        cdef size_t size

        with nogil:
            size = vxl_write(buff, self.mapbuf, 0, MAP_XY)

        data = buff[:size]
        free(buff)

        t2 = time.monotonic()

        if t2 - t1 > 1.0:
            print('VXLData.generate() took {:.2f} s'.format(t2 - t1))

        return data

    def get_encoder(self):
        return MapEncoder(self)

    # Callbacks for subclasses to implement custom per-class behavior

    async def on_map_loaded(self, protocol):
        pass

    async def on_map_unloaded(self, protocol):
        pass
