# Copyright © 2011–2012 Mathias Kaerlev
# Copyright © 2012 duckslingers
# Copyright © 2018–2019 NotAFile
# Copyright © 2021 MuffinTastic

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

from pyspades.vxl cimport VXLData, MapBuf
from pyspades.common cimport Vertex3

cdef extern from "common_c.h":
    struct LongVector:
        int x, y, z
    struct Vector:
        float x, y, z

cdef extern from "world_c.cpp":
    enum:
        CUBE_ARRAY_LENGTH
    int c_validate_hit "validate_hit" (
        float shooter_x, float shooter_y, float shooter_z,
        float orientation_x, float orientation_y, float orientation_z,
        float victim_x, float victim_y, float victim_z, float aim_tolerance, float dist_tolerance)
    int c_can_see "can_see" (MapBuf *, float x0, float y0, float z0,
        float x1, float y1, float z1)
    int c_cast_ray "cast_ray" (MapBuf *, float x0, float y0, float z0,
        float x1, float y1, float z1, float length, long* x, long* y, long* z)
    size_t cube_line_c "cube_line"(int, int, int, int, int, int, LongVector *)
    void set_globals(MapBuf *, float total_time, float dt)
    struct PlayerType:
        Vector p, e, v, s, h, f
        int mf, mb, ml, mr
        int jump, crouch, sneak
        int airborne, wade, alive, sprint
        int primary_fire, secondary_fire, weapon
        float lastclimb

    struct GrenadeType:
        Vector p, v
    PlayerType * create_player()
    void destroy_player(PlayerType * player)
    void destroy_grenade(GrenadeType * player)
    void update_timer(float value, float dt)
    void reorient_player(PlayerType * p, Vector * vector)
    int move_player(PlayerType * p)
    int try_uncrouch(PlayerType * p)
    GrenadeType * create_grenade(Vector * p, Vector * v)
    int move_grenade(GrenadeType * grenade)

cdef inline bint can_see(VXLData map, float x1, float y1, float z1,
    float x2, float y2, float z2):
    return c_can_see(map.mapbuf, x1, y1, z1, x2, y2, z2)

cdef inline bint cast_ray(VXLData map, float x1, float y1, float z1,
    float x2, float y2, float z2, float length, long* x, long* y, long* z):
    return c_cast_ray(map.mapbuf, x1, y1, z1, x2, y2, z2, length, x, y, z)

cdef class Object:
    cdef public:
        object name
        World world

    cdef int update(self, double dt) except -1

cdef class Character(Object):
    cdef:
        PlayerType * player
    cdef public:
        Vertex3 position, orientation, velocity
        object fall_callback

    cpdef int can_see(self, float x, float y, float z)
    cpdef cast_ray(self, length = ?)

    cdef int update(self, double dt) except -1

cdef class Grenade(Object):
    cdef public:
        Vertex3 position, velocity
        float fuse
        object callback
        object team
    cdef GrenadeType * grenade

    cdef int hit_test(self, Vertex3 position)

    cpdef get_next_collision(self, double dt)

    cpdef double get_damage(self, Vertex3 player_position)

    cdef int update(self, double dt) except -1

cdef class World(object):
    cdef public:
        VXLData map
        list objects
        float time

    cpdef delete_object(self, Object item)
