# Copyright © 2011–2012 Mathias Kaerlev
# Copyright © 2012 Ben Aksoy
# Copyright © 2017–2019 NotAFile

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
This module contains the definitions and registrations for the various packets used in the server
"""

# Notes:
# Here Packets are registered with the discouraged register_packet() notation.
# This is due to these packets all being cdef. This means you can not assign to
# them, and hence not use decorators on them.
#
# Other things that should probably be done here is using cython.freelist(n) to
# speed up allocation for packets

from pyspades.common import encode, decode
from pyspades.constants import NEUTRAL_TEAM, CTF_MODE, TC_MODE
from pyspades.loaders cimport Loader
from pyspades.bytes cimport ByteReader, ByteWriter
from pyspades.packet import register_packet

cimport cython

cdef inline float limit(float a):
    if a > 512.0:
        return 512.0
    elif a < 0.0:
        return 0.0
    return a

cdef inline void read_position(ByteReader reader, float * x, float * y, float * z):
    x[0] = reader.readFloat32LE()
    y[0] = reader.readFloat32LE()
    z[0] = reader.readFloat32LE()

cdef inline void write_position(ByteWriter writer, float x, float y, float z):
    writer.writeFloat32LE(x)
    writer.writeFloat32LE(y)
    writer.writeFloat32LE(z)

cdef inline unsigned int read_color(ByteReader reader):
    cdef unsigned char r, g, b
    b = reader.readUInt8LE()
    g = reader.readUInt8LE()
    r = reader.readUInt8LE()
    return (b | (g << 8) | (r << 16))

cdef inline void write_color(ByteWriter writer, unsigned int value):
    writer.writeUInt8LE(value & 0xFF)
    writer.writeUInt8LE((value >> 8) & 0xFF)
    writer.writeUInt8LE((value >> 16) & 0xFF)

import itertools
id_iter = itertools.count()

cdef class PositionData(Loader):
    id = 0

    cdef public:
        float x, y, z

    def set(self, pos):
        cdef float x, y, z
        x, y, z = pos
        self.x = x
        self.y = y
        self.z = z

    cpdef read(self, ByteReader reader):
        read_position(reader, &self.x, &self.y, &self.z)

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        write_position(writer, self.x, self.y, self.z)

register_packet(PositionData)

cdef class OrientationData(Loader):
    id = 1

    cdef public:
        float x, y, z

    def set(self, pos):
        cdef float x, y, z
        x, y, z = pos
        self.x = x
        self.y = y
        self.z = z

    cpdef read(self, ByteReader reader):
        self.x = reader.readFloat32LE()
        self.y = reader.readFloat32LE()
        self.z = reader.readFloat32LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeFloat32LE(self.x)
        writer.writeFloat32LE(self.y)
        writer.writeFloat32LE(self.z)

register_packet(OrientationData)

cdef class WorldUpdate(Loader):
    id = 2

    cdef public:
        list items

    cpdef read(self, ByteReader reader):
        cdef list items = []
        self.items = items
        for _ in range(32):
            p_x = reader.readFloat32LE()
            p_y = reader.readFloat32LE()
            p_z = reader.readFloat32LE()
            o_x = reader.readFloat32LE()
            o_y = reader.readFloat32LE()
            o_z = reader.readFloat32LE()
            items.append(((p_x, p_y, p_z), (o_x, o_y, o_z)))

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        cdef tuple item
        for item in self.items:
            (p_x, p_y, p_z), (o_x, o_y, o_z) = item
            writer.writeFloat32LE(p_x)
            writer.writeFloat32LE(p_y)
            writer.writeFloat32LE(p_z)
            writer.writeFloat32LE(o_x)
            writer.writeFloat32LE(o_y)
            writer.writeFloat32LE(o_z)

register_packet(WorldUpdate)

cdef class InputData(Loader):
    id = 3
    cdef public:
        int player_id
        bint up, down, left, right, jump, crouch, sneak, sprint

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        cdef int firstByte = reader.readUInt8LE()
        self.up = (firstByte >> 0) & 1
        self.down = (firstByte >> 1) & 1
        self.left = (firstByte >> 2) & 1
        self.right = (firstByte >> 3) & 1
        self.jump = (firstByte >> 4) & 1
        self.crouch = (firstByte >> 5) & 1
        self.sneak = (firstByte >> 6) & 1
        self.sprint = (firstByte >> 7) & 1

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        cdef int byte
        byte = (self.up | (self.down << 1) | (self.left << 2) |
            (self.right << 3) | (self.jump << 4) | (self.crouch << 5) |
            (self.sneak << 6) | (self.sprint << 7))
        writer.writeUInt8LE(byte)

register_packet(InputData)

cdef class WeaponInput(Loader):
    id = 4

    cdef public:
        bint primary, secondary
        int player_id

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        cdef unsigned char byte = reader.readUInt8LE()
        self.primary = byte & 1
        self.secondary = (byte >> 1) & 1

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        cdef unsigned char byte = self.primary | (self.secondary << 1)
        writer.writeUInt8LE(byte)


register_packet(WeaponInput)

cdef class HitPacket(Loader):
    id = 5

    cdef public:
        int player_id, value

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.value = reader.readUInt8LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        writer.writeUInt8LE(self.value)

register_packet(HitPacket, server=False)

cdef class SetHP(Loader):
    id = 5

    cdef public:
        int hp, not_fall
        float source_x, source_y, source_z

    cpdef read(self, ByteReader reader):
        self.hp = reader.readUInt8LE()
        # FALL = 0, WEAPON = 1
        self.not_fall = reader.readUInt8LE()
        self.source_x = reader.readFloat32LE()
        self.source_y = reader.readFloat32LE()
        self.source_z = reader.readFloat32LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.hp)
        writer.writeUInt8LE(self.not_fall)
        writer.writeFloat32LE(self.source_x)
        writer.writeFloat32LE(self.source_y)
        writer.writeFloat32LE(self.source_z)

register_packet(SetHP, client=False)

cdef class GrenadePacket(Loader):
    id = 6

    cdef public:
        int player_id
        float value
        tuple position, velocity

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.value = reader.readFloat32LE()
        self.position = (reader.readFloat32LE(), reader.readFloat32LE(), reader.readFloat32LE())
        self.velocity = (reader.readFloat32LE(), reader.readFloat32LE(), reader.readFloat32LE())

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        writer.writeFloat32LE(self.value)
        for value in self.position:
            writer.writeFloat32LE(value)
        for value in self.velocity:
            writer.writeFloat32LE(value)

register_packet(GrenadePacket)

@cython.freelist(8)
cdef class SetTool(Loader):
    id = 7

    cdef public:
        int player_id, value

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.value = reader.readUInt8LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        writer.writeUInt8LE(self.value)

register_packet(SetTool)

cdef class SetColor(Loader):
    id = 8

    cdef public:
        unsigned int value, player_id

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.value = read_color(reader)

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        write_color(writer, self.value)

register_packet(SetColor)

cdef class ExistingPlayer(Loader):
    id = 9

    cdef public:
        int player_id, team, weapon, tool, kills
        unsigned int color
        object name

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.team = reader.readInt8LE()
        self.weapon = reader.readUInt8LE()
        self.tool = reader.readUInt8LE()
        self.kills = reader.readUInt32LE()
        self.color = read_color(reader)
        self.name = decode(reader.readString()) # 16 bytes

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        writer.writeInt8LE(self.team)
        writer.writeUInt8LE(self.weapon)
        writer.writeUInt8LE(self.tool)
        writer.writeUInt32LE(self.kills)
        write_color(writer, self.color)
        writer.writeString(encode(self.name))

register_packet(ExistingPlayer)

cdef class ShortPlayerData(Loader):
    id = 10

    cdef public:
        int player_id, team, weapon

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.team = reader.readInt8LE()
        self.weapon = reader.readUInt8LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        writer.writeInt8LE(self.team)
        writer.writeUInt8LE(self.weapon)

register_packet(ShortPlayerData)

cdef class MoveObject(Loader):
    id = 11

    cdef public:
        unsigned int object_type, state
        float x, y, z

    cpdef read(self, ByteReader reader):
        self.object_type = reader.readUInt8LE()
        self.state = reader.readUInt8LE()
        self.x = reader.readFloat32LE()
        self.y = reader.readFloat32LE()
        self.z = reader.readFloat32LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.object_type)
        writer.writeUInt8LE(self.state)
        writer.writeFloat32LE(self.x)
        writer.writeFloat32LE(self.y)
        writer.writeFloat32LE(self.z)

register_packet(MoveObject)

@cython.freelist(8)
cdef class CreatePlayer(Loader):
    id = 12

    cdef public:
        unsigned int player_id, weapon
        int team
        float x, y, z
        object name

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.weapon = reader.readUInt8LE()
        self.team = reader.readInt8LE()
        read_position(reader, &self.x, &self.y, &self.z)
        self.name = decode(reader.readString())

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        writer.writeUInt8LE(self.weapon)
        writer.writeInt8LE(self.team)
        write_position(writer, self.x, self.y, self.z)
        writer.writeString(encode(self.name))

register_packet(CreatePlayer)

cdef class BlockAction(Loader):
    id = 13

    cdef public:
        int x, y, z, value, player_id

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.value = reader.readUInt8LE()
        self.x = reader.readInt32LE()
        self.y = reader.readInt32LE()
        self.z = reader.readInt32LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        writer.writeUInt8LE(self.value)
        writer.writeInt32LE(self.x)
        writer.writeInt32LE(self.y)
        writer.writeInt32LE(self.z)

register_packet(BlockAction)

cdef class BlockLine(Loader):
    id = 14

    cdef public:
        int player_id
        int x1, y1, z1
        int x2, y2, z2

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.x1 = reader.readInt32LE()
        self.y1 = reader.readInt32LE()
        self.z1 = reader.readInt32LE()
        self.x2 = reader.readInt32LE()
        self.y2 = reader.readInt32LE()
        self.z2 = reader.readInt32LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        writer.writeInt32LE(self.x1)
        writer.writeInt32LE(self.y1)
        writer.writeInt32LE(self.z1)
        writer.writeInt32LE(self.x2)
        writer.writeInt32LE(self.y2)
        writer.writeInt32LE(self.z2)

register_packet(BlockLine)

cdef class CTFState(Loader):
    id = CTF_MODE # this is not a real a packet, it sent as part of the StateData packet
                  # data

    cdef public:
        unsigned int team1_score, team2_score, cap_limit
        bint team1_has_intel, team2_has_intel
        unsigned int team1_carrier, team2_carrier
        float team1_flag_x, team1_flag_y, team1_flag_z
        float team2_flag_x, team2_flag_y, team2_flag_z
        float team1_base_x, team1_base_y, team1_base_z
        float team2_base_x, team2_base_y, team2_base_z

    cpdef read(self, ByteReader reader):
        self.team1_score = reader.readUInt8LE()
        self.team2_score = reader.readUInt8LE()
        self.cap_limit = reader.readUInt8LE()

        cdef int intel_flags = reader.readUInt8LE()
        self.team1_has_intel = intel_flags & 1
        self.team2_has_intel = (intel_flags >> 1) & 1

        if self.team2_has_intel:
            self.team1_carrier = reader.readUInt8LE()
            reader.skipBytes(11)
        else:
            read_position(reader, &self.team1_flag_x, &self.team1_flag_y,
                &self.team1_flag_z)

        if self.team1_has_intel:
            self.team2_carrier = reader.readUInt8LE()
            reader.skipBytes(11)
        else:
            read_position(reader, &self.team2_flag_x, &self.team2_flag_y,
                &self.team2_flag_z)

        read_position(reader, &self.team1_base_x, &self.team1_base_y,
            &self.team1_base_z)

        read_position(reader, &self.team2_base_x, &self.team2_base_y,
            &self.team2_base_z)

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.team1_score)
        writer.writeUInt8LE(self.team2_score)
        writer.writeUInt8LE(self.cap_limit)
        cdef int intel_flags = (self.team1_has_intel | (
            self.team2_has_intel << 1))

        writer.writeUInt8LE(intel_flags)

        if self.team2_has_intel:
            writer.writeUInt8LE(self.team1_carrier)
            writer.pad(11)
        else:
            write_position(writer, self.team1_flag_x, self.team1_flag_y,
                self.team1_flag_z)

        if self.team1_has_intel:
            writer.writeUInt8LE(self.team2_carrier)
            writer.pad(11)
        else:
            write_position(writer, self.team2_flag_x, self.team2_flag_y,
                self.team2_flag_z)

        write_position(writer, self.team1_base_x, self.team1_base_y,
            self.team1_base_z)

        write_position(writer, self.team2_base_x, self.team2_base_y,
            self.team2_base_z)

DEF MAX_TERRITORIES = 16
DEF TERRITORY_SIZE = 4*3+1
DEF TERRITORY_DATA = MAX_TERRITORIES * TERRITORY_SIZE

cdef class Territory(Loader):
    cdef public:
        float x, y, z
        unsigned int state

    cpdef read(self, ByteReader reader):
        read_position(reader, &self.x, &self.y, &self.z)
        self.state = reader.readUInt8LE()

    cpdef write(self, ByteWriter writer):
        write_position(writer, self.x, self.y, self.z)
        writer.writeUInt8LE(self.state)

cdef class ObjectTerritory(Loader):
    cdef public:
        object item

    cpdef write(self, ByteWriter writer):
        write_position(writer, self.item.x, self.item.y, self.item.z)
        team = self.item.team
        cdef int state
        if team is None:
            state = NEUTRAL_TEAM
        else:
            state = team.id
        writer.writeUInt8LE(state)

cdef class TCState(Loader):
    id = TC_MODE # this is not a real a packet, it sent as part of the StateData packet
                 # data

    cdef public:
        list territories

    cpdef read(self, ByteReader reader):
        self.territories = []
        cdef unsigned int count = reader.readUInt8LE()
        for _ in range(count):
            self.territories.append(Territory(reader))

    def set_entities(self, items):
        self.territories = []
        for item in items:
            territory = ObjectTerritory()
            territory.item = item
            self.territories.append(territory)

    cpdef write(self, ByteWriter writer):
        cdef Loader territory
        writer.writeUInt8LE(len(self.territories))
        for territory in self.territories:
            territory.write(writer)
        writer.pad((MAX_TERRITORIES - len(self.territories)) * TERRITORY_SIZE)

modes = {
    CTF_MODE : CTFState,
    TC_MODE : TCState
}

cdef inline tuple read_team_color(ByteReader reader):
    b = reader.readUInt8LE()
    g = reader.readUInt8LE()
    r = reader.readUInt8LE()
    return (r, g, b)

cdef inline void write_team_color(ByteWriter writer, tuple color):
    r, g, b = color
    writer.writeUInt8LE(b)
    writer.writeUInt8LE(g)
    writer.writeUInt8LE(r)

cdef class StateData(Loader):
    id = 15

    cdef public:
        int player_id
        tuple fog_color
        tuple team1_color
        tuple team2_color
        Loader state
        object team1_name, team2_name

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.fog_color = read_team_color(reader)
        self.team1_color = read_team_color(reader)
        self.team2_color = read_team_color(reader)
        self.team1_name = decode(reader.readString(10))
        self.team2_name = decode(reader.readString(10))
        cdef int mode = reader.readUInt8LE()
        self.state = modes[mode](reader)

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        write_team_color(writer, self.fog_color)
        write_team_color(writer, self.team1_color)
        write_team_color(writer, self.team2_color)
        writer.writeString(encode(self.team1_name), 10)
        writer.writeString(encode(self.team2_name), 10)
        writer.writeUInt8LE(self.state.id)
        self.state.write(writer)

register_packet(StateData)

cdef class KillAction(Loader):
    id = 16

    cdef public:
        int player_id, killer_id, kill_type, respawn_time

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.killer_id = reader.readUInt8LE()
        self.kill_type = reader.readUInt8LE()
        self.respawn_time = reader.readUInt8LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        writer.writeUInt8LE(self.killer_id)
        writer.writeUInt8LE(self.kill_type)
        writer.writeUInt8LE(self.respawn_time)

register_packet(KillAction)

cdef class ChatMessage(Loader):
    id = 17

    cdef public:
        unsigned int player_id, chat_type
        object value

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.chat_type = reader.readUInt8LE()
        self.value = decode(reader.readString())

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        writer.writeUInt8LE(self.chat_type)
        writer.writeString(encode(self.value))

register_packet(ChatMessage)

cdef class MapStart(Loader):
    id = 18

    cdef public:
        unsigned int size

    cpdef read(self, ByteReader reader):
        self.size = reader.readUInt32LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt32LE(self.size)

register_packet(MapStart)

cdef class MapChunk(Loader):
    id = 19

    cdef public:
        object data

    cpdef read(self, ByteReader reader):
        self.data = reader.read()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.write(self.data)

register_packet(MapChunk)

@cython.freelist(8)
cdef class PlayerLeft(Loader):
    id = 20

    cdef public:
        int player_id

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)

register_packet(PlayerLeft)

cdef class TerritoryCapture(Loader):
    id = 21

    cdef public:
        unsigned int object_index, winning, state

    cpdef read(self, ByteReader reader):
        self.object_index = reader.readUInt8LE()
        self.winning = reader.readUInt8LE()
        self.state = reader.readUInt8LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.object_index)
        writer.writeUInt8LE(self.winning)
        writer.writeUInt8LE(self.state)

register_packet(TerritoryCapture)

cdef class ProgressBar(Loader):
    id = 22

    cdef public:
        unsigned int object_index, capturing_team
        int rate
        float progress

    cpdef read(self, ByteReader reader):
        self.object_index = reader.readUInt8LE()
        self.capturing_team = reader.readInt8LE()
        self.rate = reader.readInt8LE()
        self.progress = reader.readFloat32LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.object_index)
        writer.writeInt8LE(self.capturing_team)
        writer.writeInt8LE(self.rate)
        writer.writeFloat32LE(self.progress)

register_packet(ProgressBar)

@cython.freelist(8)
cdef class IntelCapture(Loader):
    id = 23

    cdef public:
        int player_id
        bint winning

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.winning = reader.readUInt8LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        writer.writeUInt8LE(self.winning)

register_packet(IntelCapture)

cdef class IntelPickup(Loader):
    id = 24

    cdef public:
        int player_id

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)

register_packet(IntelPickup)

cdef class IntelDrop(Loader):
    id = 25

    cdef public:
        int player_id
        float x, y, z

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        read_position(reader, &self.x, &self.y, &self.z)

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        write_position(writer, self.x, self.y, self.z)

register_packet(IntelDrop)

cdef class Restock(Loader):
    id = 26

    cdef public:
        int player_id

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)

register_packet(Restock)

@cython.freelist(8)
cdef class FogColor(Loader):
    id = 27

    cdef public:
        int color

    cpdef read(self, ByteReader reader):
        self.color = reader.readUInt32LE() >> 8

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt32LE(self.color << 8)

register_packet(FogColor)

@cython.freelist(8)
cdef class WeaponReload(Loader):
    id = 28

    cdef public:
        int player_id, clip_ammo, reserve_ammo

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.clip_ammo = reader.readUInt8LE()
        self.reserve_ammo = reader.readUInt8LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        writer.writeUInt8LE(self.clip_ammo)
        writer.writeUInt8LE(self.reserve_ammo)

register_packet(WeaponReload)

cdef class ChangeTeam(Loader):
    id = 29

    cdef public:
        int player_id, team

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.team = reader.readInt8LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        writer.writeInt8LE(self.team)

register_packet(ChangeTeam)

cdef class ChangeWeapon(Loader):
    id = 30

    cdef public:
        int player_id, weapon

    cpdef read(self, ByteReader reader):
        self.player_id = reader.readUInt8LE()
        self.weapon = reader.readUInt8LE()

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt8LE(self.player_id)
        writer.writeUInt8LE(self.weapon)

register_packet(ChangeWeapon)

cdef class HandShakeInit(Loader):
    id = 31

    cpdef read(self, ByteReader reader):
        pass

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)
        writer.writeUInt32LE(42)

register_packet(HandShakeInit)

cdef class HandShakeReturn(Loader):
    id = 32

    cdef public:
        int success

    cpdef read(self, ByteReader reader):
        self.success = int(reader.readUInt32LE() == 42)

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)

register_packet(HandShakeReturn)

cdef class VersionRequest(Loader):
    id = 33

    cpdef read(self, ByteReader reader):
        pass

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)

register_packet(VersionRequest)

cdef class VersionResponse(Loader):
    id = 34

    cdef public:
        str client
        tuple version
        str os_info

    cpdef read(self, ByteReader reader):
        magic_no = reader.readChar()
        self.client = chr(magic_no)
        self.version = (
            reader.readUInt8LE(),
            reader.readUInt8LE(),
            reader.readUInt8LE(),
        )
        self.os_info = decode(reader.readString())

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)

register_packet(VersionResponse)


cdef class ProtocolExtensionInfo(Loader):
    """packet used to exchange the list of supported protocol extensions between
    server and client

    extensions is a list of (extension_id, version) tuples
    """
    id = 60

    cdef public:
        list extensions

    cpdef read(self, ByteReader reader):
        extension_count = reader.readUInt8LE()
        extensions = []
        for _ in range(extension_count):
            extensions.append(
                (reader.readUInt8LE(), reader.readUInt8LE())
            )
        self.extensions = extensions

    cpdef write(self, ByteWriter writer):
        writer.writeUInt8LE(self.id)

        writer.writeUInt8LE(len(self.extensions))

        for ext in self.extensions:
            writer.writeUInt8LE(ext[0])
            writer.writeUInt8LE(ext[1])

register_packet(ProtocolExtensionInfo)
