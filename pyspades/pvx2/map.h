/*
    Copyright © 2026 notaburner <davidco7777@protonmail.com>
    Copyright © 2026 rzrn

    Permission to use, copy, modify, and/or distribute this software for any purpose
    with or without fee is hereby granted.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
    REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
    FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
    INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
    OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
    TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
    THIS SOFTWARE.
*/

#ifndef PVX2_MAP_H
#define PVX2_MAP_H

#define MAP_X 512
#define MAP_Y 512
#define MAP_Z  64

#define MAP_XY  (MAP_X * MAP_Y)
#define MAP_XYZ (MAP_X * MAP_Y * MAP_Z)

typedef unsigned long long MapCol;

struct MapBuf {
    /* Compiler abuse? */
    unsigned char * colorbuf;
    MapCol          solidbuf[MAP_XY];
};

#ifndef __has_attribute
    #define map_has_attribute(x) 0
#else
    #define map_has_attribute(x) __has_attribute(x)
#endif

#if map_has_attribute(always_inline)
    #define MapApi static inline __attribute__((always_inline))
#else
    #define MapApi static inline
#endif

/* Base data access */
MapApi long map_to_xy(int x, int y);

MapApi MapCol map_get_col_xy(struct MapBuf *, long      xy);
MapApi MapCol map_get_col   (struct MapBuf *, int x, int y);
MapApi void   map_set_col_xy(struct MapBuf *, long      xy, MapCol val);
MapApi void   map_set_col   (struct MapBuf *, int x, int y, MapCol val);

MapApi unsigned char * map_get_color_xy(struct MapBuf *, long      xy, int z);
MapApi unsigned char * map_get_color   (struct MapBuf *, int x, int y, int z);

MapApi void map_set_color_xy(
    struct MapBuf *, long      xy, int z, const void * colors, long len
);
MapApi void map_set_color(
    struct MapBuf *, int x, int y, int z, const void * colors, long len
);

/* Whole-map solidity manipulation */
MapApi void map_clear(struct MapBuf *);
MapApi void map_copy (struct MapBuf *, const struct MapBuf *);

/* Column solidity manipulation */
MapApi void map_create_range_xy (struct MapBuf *, long      xy, int z, long len);
MapApi void map_create_range    (struct MapBuf *, int x, int y, int z, long len);
MapApi void map_create_xy       (struct MapBuf *, long      xy, int z);
MapApi void map_create          (struct MapBuf *, int x, int y, int z);

MapApi void map_destroy_range_xy(struct MapBuf *, long      xy, int z, long len);
MapApi void map_destroy_range   (struct MapBuf *, int x, int y, int z, long len);
MapApi void map_destroy_xy      (struct MapBuf *, long      xy, int z);
MapApi void map_destroy         (struct MapBuf *, int x, int y, int z);

/* Column solidity querying */
MapApi MapCol map_is_solid_range_xy (struct MapBuf *, long      xy, int z, long len);
MapApi MapCol map_is_solid_range    (struct MapBuf *, int x, int y, int z, long len);
MapApi int    map_is_solid_xy       (struct MapBuf *, long      xy, int z);
MapApi int    map_is_solid          (struct MapBuf *, int x, int y, int z);

MapApi int    map_count_solids_xy   (struct MapBuf *, long      xy, int z);
MapApi int    map_count_solids      (struct MapBuf *, int x, int y, int z);
MapApi int    map_count_nonsolids_xy(struct MapBuf *, long      xy, int z);
MapApi int    map_count_nonsolids   (struct MapBuf *, int x, int y, int z);

MapApi int map_get_floor_xy(struct MapBuf *, long      xy, int z);
MapApi int map_get_floor   (struct MapBuf *, int x, int y, int z);
MapApi int map_get_top_xy  (struct MapBuf *, long      xy);
MapApi int map_get_top     (struct MapBuf *, int x, int y);

#include <stdlib.h>
#include <string.h>

#if map_has_attribute(assume)
    #define map_affirm(x) __attribute__((assume(x)))
#else
    #define map_affirm(x) do {} while (0)
#endif

#define map_affirm_xy()    map_affirm(xy >= 0 && xy < MAP_XY)
#define map_affirm_x_y()   map_affirm(x >= 0 && x < MAP_X && y >= 0 && y < MAP_Y)
#define map_affirm_z()     map_affirm(z >= 0 && z < MAP_Z)
#define map_affirm_z_len() map_affirm(z >= 0 && len >= 0 && z + len <= MAP_Z)

#define map_affirm_xy_z()      do {map_affirm_xy();  map_affirm_z();} while (0)
#define map_affirm_x_y_z()     do {map_affirm_x_y(); map_affirm_z();} while (0)
#define map_affirm_xy_z_len()  do {map_affirm_xy();  map_affirm_z_len();} while (0)
#define map_affirm_x_y_z_len() do {map_affirm_x_y(); map_affirm_z_len();} while (0)

/* These don't invoke undefined behavior if y >= MAP_Z. */
#define col_shl(x, y) ((x) << ((y) & (MAP_Z - 1)))
#define col_shr(x, y) ((x) >> ((y) & (MAP_Z - 1)))

#ifndef __has_builtin
    #define map_has_builtin(x) 0
#else
    #define map_has_builtin(x) __has_builtin(x)
#endif

#if map_has_builtin(__builtin_popcountll)
    #define map_col_popcnt(x) __builtin_popcountll(x)
#else
    #define M1  0x5555555555555555LLU
    #define M2  0x3333333333333333LLU
    #define M4  0x0F0F0F0F0F0F0F0FLLU
    #define H01 0x0101010101010101LLU

    // We assume that `x` fits into 64 bits
    MapApi int map_col_popcnt(MapCol x) {
        // https://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
        x -= (x >> 1) & M1;
        x = (x & M2) + ((x >> 2) & M2);
        x = (x + (x >> 4)) & M4;
        return (x * H01) >> 56;
    }

    #undef H01
    #undef M4
    #undef M2
    #undef M1
#endif

#if map_has_builtin(__builtin_ctzg)
    #define map_col_ctz(x, y) __builtin_ctzg(x, y)
#elif map_has_builtin(__builtin_ctzll)
    MapApi int map_col_ctz(MapCol x, int y) {
        return x == 0 ? y : __builtin_ctzll(x);
    }
#else
    #define D64 0x03F79D71B4CB0A89ULL

    MapApi int map_col_ctz(MapCol x, int y) {
        // https://en.wikipedia.org/wiki/Find_first_set#CTZ
        // https://web.archive.org/web/20200109231233/https://www.chessprogramming.org/BitScan#With_separated_LS1B

        static int ctz[64] = {
            0, 47,  1, 56, 48, 27,  2, 60,
           57, 49, 41, 37, 28, 16,  3, 61,
           54, 58, 35, 52, 50, 42, 21, 44,
           38, 32, 29, 23, 17, 11,  4, 62,
           46, 55, 26, 59, 40, 36, 15, 53,
           34, 51, 20, 43, 31, 22, 10, 45,
           25, 39, 14, 33, 19, 30,  9, 24,
           13, 18,  8, 12,  7,  6,  5, 63
        };

        return x == 0 ? y : ctz[((x ^ (x - 1)) * D64) >> 58];
    }

    #undef D64
#endif

#if map_has_builtin(__builtin_clzll)
    #define map_col_clz(x) __builtin_clzll(x)
#else
    MapApi int map_col_clz(MapCol x) {
        // https://en.wikipedia.org/wiki/Find_first_set#CLZ

        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        x |= x >> 32;

        return 64 - map_col_ctz(~x, 64);
    }
#endif

/* Lookup table is a little faster than handling << 64 specially. */
#define B(x) ((1LLU << (x)) - 1)
static const unsigned long long bittbl[65] = {
    B( 0), B( 1), B( 2), B( 3),
    B( 4), B( 5), B( 6), B( 7),
    B( 8), B( 9), B(10), B(11),
    B(12), B(13), B(14), B(15),
    B(16), B(17), B(18), B(19),
    B(20), B(21), B(22), B(23),
    B(24), B(25), B(26), B(27),
    B(28), B(29), B(30), B(31),
    B(32), B(33), B(34), B(35),
    B(36), B(37), B(38), B(39),
    B(40), B(41), B(42), B(43),
    B(44), B(45), B(46), B(47),
    B(48), B(49), B(50), B(51),
    B(52), B(53), B(54), B(55),
    B(56), B(57), B(58), B(59),
    B(60), B(61), B(62), B(63),
    ~0LLU
};
#undef B

/*
 * Base data access
 */
MapApi long map_to_xy(int x, int y) {
    map_affirm_x_y();
    return y * MAP_X + x;
}

MapApi MapCol map_get_col_xy(struct MapBuf * map, long xy) {
    map_affirm_xy();
    return map->solidbuf[xy];
}

MapApi MapCol map_get_col(struct MapBuf * map, int x, int y) {
    map_affirm_x_y();
    return map_get_col_xy(map, map_to_xy(x, y));
}

MapApi void map_set_col_xy(struct MapBuf * map, long xy, MapCol val) {
    map_affirm_xy();
    map->solidbuf[xy] = val;
}

MapApi void map_set_col(struct MapBuf * map, int x, int y, MapCol val) {
    map_affirm_x_y();
    map_set_col_xy(map, map_to_xy(x, y), val);
}

MapApi unsigned char * map_get_color_xy(struct MapBuf * map, long xy, int z) {
    map_affirm_xy_z();
    return map->colorbuf + (xy * MAP_Z + z) * 3;
}

MapApi unsigned char * map_get_color(struct MapBuf * map, int x, int y, int z) {
    map_affirm_x_y_z();
    return map_get_color_xy(map, map_to_xy(x, y), z);
}

MapApi void map_set_color_xy(
    struct MapBuf * map, long xy, int z, const void * colors, long len
) {
    /* Affirming z seems to hurt more than it helps here. */
    map_affirm_xy();
    map_affirm(len >= 0 && len <= 64);
    memcpy(
        map->colorbuf + (xy * MAP_Z + z) * 3,
        colors,
        len * 3
    );
}

MapApi void map_set_color(
    struct MapBuf * map, int x, int y, int z, const void * colors, long len
) {
    map_affirm_x_y();
    map_affirm(len >= 0 && len <= 64);
    memcpy(
        map->colorbuf + (map_to_xy(x, y) * MAP_Z + z) * 3,
        colors,
        len * 3
    );
}

/*
 * Whole-map solidity manipulation
 */
MapApi void map_clear(struct MapBuf * map) {
    memset(map->solidbuf, 0, sizeof(MapCol) * MAP_XY);
}

MapApi void map_copy(struct MapBuf * dst, const struct MapBuf * src) {
    memcpy(dst->solidbuf, src->solidbuf, sizeof(dst->solidbuf));

    if (dst->colorbuf != NULL && src->colorbuf != NULL)
        memcpy(dst->colorbuf, src->colorbuf, 3 * MAP_XYZ);
}

/*
 * Column solidity manipulation
 */
MapApi void map_create_range_xy(struct MapBuf * map, long xy, int z, long len) {
    map_affirm_xy_z_len();
    map_set_col_xy(map, xy,
        map_get_col_xy(map, xy) | col_shl(bittbl[len], z)
    );
}

MapApi void map_create_range(struct MapBuf * map, int x, int y, int z, long len) {
    map_affirm_x_y_z_len();
    map_create_range_xy(map, map_to_xy(x, y), z, len);
}

MapApi void map_create_xy(struct MapBuf * map, long xy, int z) {
    map_affirm_xy_z();
    map_create_range_xy(map, xy, z, 1);
}

MapApi void map_create(struct MapBuf * map, int x, int y, int z) {
    map_affirm_x_y_z();
    map_create_range(map, x, y, z, 1);
}

MapApi void map_destroy_range_xy(struct MapBuf * map, long xy, int z, long len) {
    map_affirm_xy_z_len();
    map_set_col_xy(map, xy,
        map_get_col_xy(map, xy) & ~col_shl(bittbl[len], z)
    );
}

MapApi void map_destroy_range(struct MapBuf * map, int x, int y, int z, long len) {
    map_affirm_x_y_z_len();
    map_destroy_range_xy(map, map_to_xy(x, y), z, len);
}

MapApi void map_destroy_xy(struct MapBuf * map, long xy, int z) {
    map_affirm_xy_z();
    map_destroy_range_xy(map, xy, z, 1);
}

MapApi void map_destroy(struct MapBuf * map, int x, int y, int z) {
    map_affirm_x_y_z();
    map_destroy_range(map, x, y, z, 1);
}

/*
 * Column solidity querying
 */
MapApi MapCol map_is_solid_range_xy(struct MapBuf * map, long xy, int z, long len) {
    map_affirm_xy_z_len();
    return col_shr(map_get_col_xy(map, xy), z) & bittbl[len];
}

MapApi MapCol map_is_solid_range(struct MapBuf * map, int x, int y, int z, long len) {
    map_affirm_x_y_z_len();
    return map_is_solid_range_xy(map, map_to_xy(x, y), z, len);
}

MapApi int map_is_solid_xy(struct MapBuf * map, long xy, int z) {
    map_affirm_xy_z();
    return map_is_solid_range_xy(map, xy, z, 1);
}

MapApi int map_is_solid(struct MapBuf * map, int x, int y, int z) {
    map_affirm_x_y_z();
    return map_is_solid_range(map, x, y, z, 1);
}

MapApi int map_count_solids_xy(struct MapBuf * map, long xy, int z) {
    map_affirm_xy_z();
    return map_col_ctz(~map_get_col_xy(map, xy) >> z, MAP_Z);
}

MapApi int map_count_solids(struct MapBuf * map, int x, int y, int z) {
    map_affirm_x_y_z();
    return map_count_solids_xy(map, map_to_xy(x, y), z);
}

MapApi int map_count_nonsolids_xy(struct MapBuf * map, long xy, int z) {
    map_affirm_xy_z();
    return map_col_ctz(map_get_col_xy(map, xy) >> z, MAP_Z - z);
}

MapApi int map_count_nonsolids(struct MapBuf * map, int x, int y, int z) {
    map_affirm_x_y_z();
    return map_count_nonsolids_xy(map, map_to_xy(x, y), z);
}

MapApi int map_get_floor_xy(struct MapBuf * map, long xy, int z) {
    map_affirm_xy_z();
    return z + map_count_nonsolids_xy(map, xy, z);
}

MapApi int map_get_floor(struct MapBuf * map, int x, int y, int z) {
    map_affirm_x_y_z();
    return map_get_floor_xy(map, map_to_xy(x, y), z);
}

MapApi int map_get_top_xy(struct MapBuf * map, long xy) {
    map_affirm_xy();
    return map_col_ctz(map_get_col_xy(map, xy), MAP_Z);
}

MapApi int map_get_top(struct MapBuf * map, int x, int y) {
    map_affirm_x_y();
    return map_get_top_xy(map, map_to_xy(x, y));
}

#undef col_shr
#undef col_shl

#undef map_affirm_x_y_z_len
#undef map_affirm_xy_z_len
#undef map_affirm_x_y_z
#undef map_affirm_xy_z

#undef map_affirm_z_len
#undef map_affirm_z
#undef map_affirm_x_y
#undef map_affirm_xy

#endif
