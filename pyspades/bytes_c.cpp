/*
    Copyright © 2011–2012 Mathias Kaerlev
    Copyright © 2017–2018 NotAFile
    Copyright © 2026 rzrn

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Python.h"
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

stringstream *create_stream()
{
    stringstream *ss = new stringstream(stringstream::out | stringstream::binary);
    return ss;
}

void delete_stream(stringstream *ss)
{
    delete ss;
}

/*
read methods
*/

inline char read_c8le(const char * data) {
    return data[0];
}

// byte

inline uint8_t read_u8le(const char * data) {
    return ((uint8_t *) data)[0];
}

inline int8_t read_i8le(const char * data) {
    return ((int8_t *) data)[0];
}

// short

inline uint16_t read_u16le(const char * data) {
    const uint8_t * const buff = (const uint8_t *) data;

    return (uint16_t) buff[0] << 0
         | (uint16_t) buff[1] << 8;
}

inline int16_t read_i16le(const char * data) {
    return int16_t(read_u16le(data));
}

// int

inline uint32_t read_u32le(const char * data) {
    const uint8_t * const buff = (const uint8_t *) data;

    return (uint32_t) buff[0] << 0
         | (uint32_t) buff[1] << 8
         | (uint32_t) buff[2] << 16
         | (uint32_t) buff[3] << 24;
}

inline int32_t read_i32le(const char * data) {
    return int32_t(read_u32le(data));
}

// float

inline double read_f32le(const char * data) {
    #if (PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11)
        return PyFloat_Unpack4(data, true);
    #else
        return _PyFloat_Unpack4((const unsigned char *) data, true);
    #endif
}

/*
write methods
*/

inline void write_c8le(stringstream * ss, char value) {
    ss->put(value);
}

// byte

inline void write_i8le(stringstream * ss, int8_t value) {
    ss->put(value);
}

inline void write_u8le(stringstream * ss, uint8_t value) {
    ss->put(value);
}

// short

inline void write_u16le(stringstream * ss, uint16_t value) {
    ss->put((value >> 0) & 0xFF);
    ss->put((value >> 8) & 0xFF);
}

inline void write_i16le(stringstream * ss, int16_t value) {
    write_u16le(ss, uint16_t(value));
}

// int

inline void write_u32le(stringstream * ss, uint32_t value) {
    ss->put((value >> 0)  & 0xFF);
    ss->put((value >> 8)  & 0xFF);
    ss->put((value >> 16) & 0xFF);
    ss->put((value >> 24) & 0xFF);
}

inline void write_i32le(stringstream * ss, int32_t value) {
    write_u32le(ss, uint32_t(value));
}

// float

inline void write_f32le(stringstream * ss, double value) {
    char out[4];

    #if (PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11)
        PyFloat_Pack4(value, (char *) &out, true);
    #else
        _PyFloat_Pack4(value, (unsigned char *) &out, true);
    #endif

    ss->write(out, 4);
}

inline void write_string(stringstream *ss, char *data, size_t size)
{
    ss->write(data, size);
    ss->put(0);
}

inline void write(stringstream *ss, char *data, size_t size)
{
    ss->write(data, size);
}

inline void rewind_stream(stringstream *ss, int bytes)
{
    ss->seekp(-bytes, stringstream::cur);
}

inline size_t get_stream_size(stringstream *ss)
{
    return ss->str().length();
}

inline size_t get_stream_pos(stringstream *ss)
{
    streampos pos = ss->tellp();
    if (pos == (streampos)-1)
        return 0;
    return pos;
}

inline PyObject *get_stream(stringstream *ss)
{
    const string tmp = ss->str();
    return PyBytes_FromStringAndSize(tmp.c_str(), tmp.length());
}
