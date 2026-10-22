# Copyright © 2011–2012 Mathias Kaerlev
# Copyright © 2017–2018 NotAFile
# Copyright © 2025 utf-4096
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

from libc.math cimport NAN

from libc.stdint cimport int8_t, int16_t, int32_t, uint8_t, uint16_t, uint32_t

cdef extern from "<sstream>" namespace "std":
    cdef cppclass stringstream:
        pass

cdef class ByteReader:
    cdef char * data
    cdef char * pos
    cdef char * end
    cdef int start, size
    cdef object input

    cdef char * check_available(self, int size) except NULL
    cpdef read(self, int bytecount = ?)

    cpdef int readInt8LE(self) except 128
    cpdef int readUInt8LE(self) except 256

    cpdef int readInt16LE(self) except 32768
    cpdef int readUInt16LE(self) except 65536

    cpdef long long readInt32LE(self) except 2147483648
    cpdef long long readUInt32LE(self) except 4294967296

    cpdef float readFloat32LE(self) except? NAN

    cpdef int readChar(self)
    cpdef bytes readString(self, int size = ?)

    cpdef ByteReader readReader(self, int size = ?)
    cpdef int dataLeft(self)
    cdef void _skip(self, int bytecount)
    cpdef skipBytes(self, int bytecount)
    cpdef rewind(self, int value)
    cpdef seek(self, size_t pos)
    cpdef size_t tell(self)

cdef class ByteWriter:
    cdef stringstream * stream

    cdef void writeSize(self, char * data, int size)
    cpdef write(self, data)

    cpdef writeInt8LE(self, int8_t value)
    cpdef writeUInt8LE(self, uint8_t value)

    cpdef writeInt16LE(self, int16_t value)
    cpdef writeUInt16LE(self, uint16_t value)

    cpdef writeInt32LE(self, int32_t value)
    cpdef writeUInt32LE(self, uint32_t value)

    cpdef writeFloat32LE(self, float value)

    cpdef writeChar(self, value)
    cpdef writeString(self, value, int size = ?)
    cpdef writeStringSize(self, char * value, int size)

    cpdef pad(self, int bytecount)
    cpdef rewind(self, int bytecount)
    cpdef size_t tell(self)
