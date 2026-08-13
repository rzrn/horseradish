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

"""
The ByteReader/Bytewriter classes are used to read and write various data types
from and to byte-like objects. This is used e.g. to read the contents of
packets.
"""

cdef extern from "bytes_c.cpp":
    int8_t read_i8le(const char * data)
    uint8_t read_u8le(const char * data)

    int16_t read_i16le(const char * data)
    uint16_t read_u16le(const char * data)

    int32_t read_i32le(const char * data)
    uint32_t read_u32le(const char * data)

    double read_f32le(const char *)

    char read_c8le(const char * data)
    char * read_string(char * data)

    stringstream * create_stream()
    void delete_stream(stringstream * stream)

    void write_i8le(stringstream * ss, int8_t value)
    void write_u8le(stringstream * ss, uint8_t value)

    void write_i16le(stringstream * ss, int16_t value)
    void write_u16le(stringstream * ss, uint16_t value)

    void write_i32le(stringstream * ss, int32_t value)
    void write_u32le(stringstream * ss, uint32_t value)

    void write_f32le(stringstream * ss, double value)

    void write_c8le(stringstream * ss, char value)
    void write_string(stringstream * stream, char * data, size_t size)

    void write(stringstream * stream, char * data, size_t size)
    void rewind_stream(stringstream * stream, int bytecount)
    object get_stream(stringstream * stream)
    size_t get_stream_size(stringstream * stream)
    size_t get_stream_pos(stringstream * stream)

cdef extern from "<sstream>" namespace "std":
    cdef cppclass stringstream:
        pass

class NoDataLeft(Exception):
    pass

cdef class ByteReader:
    """Reads various data types from a bytes-like object"""
    def __init__(self, input_data, int start = 0, int size = -1):
        self.input = input_data
        self.data = input_data
        self.data += start
        self.pos = self.data
        if size == -1:
            size = len(input_data) - start
        self.size = size
        self.end = self.data + size
        self.start = start

    cdef char * check_available(self, int size) except NULL:
        cdef char * data = self.pos
        if data + size > self.end:
            raise NoDataLeft('not enough data')
        self.pos += size
        return data

    cpdef read(self, int bytecount = -1):
        """read a number of bytes

        Arguments:
            bytecount (int, optional): The number of bytes to read. If omitted, all bytes available are read

        Returns:
            bytes: ``bytecount`` bytes of data
        """
        cdef int left = self.dataLeft()
        if bytecount == -1 or bytecount > left:
            bytecount = left
        ret = self.pos[:bytecount]
        self.pos += bytecount
        return ret

    cpdef int readInt8LE(self) except 128:
        """read one byte of data as signed integer

        Returns:
            int: The value of the byte as int
        """

        cdef char * pos = self.check_available(1)
        return read_i8le(pos)

    cpdef int readUInt8LE(self) except 256:
        """read one byte of data as unsigned integer

        Returns:
            int: The value of the byte as int
        """

        cdef char * pos = self.check_available(1)
        return read_u8le(pos)

    cpdef int readInt16LE(self) except 32768:
        """read two bytes of data as signed little-endian integer

        Returns:
            int: The value of the bytes as int
        """

        cdef char * pos = self.check_available(2)
        return read_i16le(pos)

    cpdef int readUInt16LE(self) except 65536:
        """read two bytes of data as unsigned little-endian integer

        Returns:
            int: The value of the bytes as int
        """

        cdef char * pos = self.check_available(2)
        return read_u16le(pos)

    cpdef long long readInt32LE(self) except 2147483648:
        """read four bytes of data as signed little-endian integer

        Returns:
            int: The value of the bytes as int
        """

        cdef char * pos = self.check_available(4)
        return read_i32le(pos)

    cpdef long long readUInt32LE(self) except 4294967296:
        """read four bytes of data as unsigned little-endian integer

        Returns:
            int: The value of the bytes as int
        """

        cdef char * pos = self.check_available(4)
        return read_u32le(pos)

    cpdef float readFloat32LE(self) except? NAN:
        """read four bytes of data as little-endian floating point number

        Returns:
            float: The value of the bytes as float
        """

        cdef char * pos = self.check_available(4)
        return read_f32le(pos)

    cpdef int readChar(self):
        """read one byte of data as character

        Returns:
            bytes: The value of the byte as character
        """

        cdef char * pos = self.check_available(1)
        return read_c8le(pos)

    cpdef bytes readString(self, int size = -1):
        """read a string

        Arguments:
            size (int): If set, read ``size`` bytes, else read all bytes available

        Returns:
            bytes: The value of the bytes
        """
        value = self.pos
        if size == -1:
            size = len(value) + 1
        if size > self.end - self.pos:
            size = self.end - self.pos
            value = value[:size]
        self.pos += size
        return bytes(value)

    cpdef ByteReader readReader(self, int size = -1):
        cdef int left = self.dataLeft()
        if size == -1 or size > left:
            size = left
        cdef ByteReader reader = ByteReader(self.input,
            (self.pos - self.data) + self.start, size)
        self.pos += size
        return reader

    cpdef size_t tell(self):
        """get the current position in the buffer

        Returns:
            int: The current position in bytes"""
        return self.pos - self.data

    cpdef int dataLeft(self):
        """get the number of bytes left in the buffer

        Returns:
            int: The number of bytes left"""
        return self.end - self.pos

    cpdef seek(self, size_t pos):
        """move to a position in the buffer

        Arguments:
            pos (int): position to seek to
        """
        self.pos = self.data + pos
        if self.pos > self.end:
            self.pos = self.end
        if self.pos < self.data:
            self.pos = self.data

    cdef void _skip(self, int bytecount):
        self.pos += bytecount
        if self.pos > self.end:
            self.pos = self.end
        if self.pos < self.data:
            self.pos = self.data

    cpdef skipBytes(self, int bytecount):
        """move the position ``bytecount`` bytes ahead

        Arguments:
            bytecount: number of bytes to move ahead
        """
        self._skip(bytecount)

    cpdef rewind(self, int value):
        """move the position ``bytecount`` bytes back

        Arguments:
            bytecount: number of bytes to move back
        """
        self._skip(-value)

    def __len__(self):
        return self.size

    def __bytes__(self):
        return self.data[:self.size]

cdef class ByteWriter:
    def __init__(self):
        self.stream = create_stream()

    cdef void writeSize(self, char * data, int size):
        write(self.stream, data, size)

    cpdef write(self, data):
        write(self.stream, data, len(data))

    cpdef writeInt8LE(self, int8_t value):
        write_i8le(self.stream, value)

    cpdef writeUInt8LE(self, uint8_t value):
        write_u8le(self.stream, value)

    cpdef writeInt16LE(self, int16_t value):
        write_i16le(self.stream, value)

    cpdef writeUInt16LE(self, uint16_t value):
        write_u16le(self.stream, value)

    cpdef writeInt32LE(self, int32_t value):
        write_i32le(self.stream, value)

    cpdef writeUInt32LE(self, uint32_t value):
        write_u32le(self.stream, value)

    cpdef writeFloat32LE(self, float value):
        write_f32le(self.stream, value)

    cpdef writeChar(self, value):
        write_c8le(self.stream, value)

    cpdef writeString(self, value, int size = -1):
        write_string(self.stream, value, len(value))
        if size != -1:
            self.pad(size - (len(value) + 1))

    cpdef writeStringSize(self, char * value, int size):
        write_string(self.stream, value, size)

    cpdef pad(self, int bytecount):
        cdef int i
        for i in range(bytecount):
            write_u8le(self.stream, 0)

    cpdef rewind(self, int bytecount):
        rewind_stream(self.stream, bytecount)

    cpdef size_t tell(self):
        return get_stream_pos(self.stream)

    def __bytes__(self):
        return get_stream(self.stream)

    def __dealloc__(self):
        delete_stream(self.stream)

    def __len__(self):
        return get_stream_size(self.stream)
