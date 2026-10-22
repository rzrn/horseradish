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

#include "write.h"
#include <string.h>

#if map_has_builtin(__builtin_expect)
    #define likely(x)   __builtin_expect(!!(x), 1)
    #define unlikely(x) __builtin_expect(!!(x), 0)
#else
    #define likely(x)   0
    #define unlikely(x) 0
#endif

MapApi void write_colors(unsigned char ** dst, struct MapBuf * map, int xy, int z, int endz) {
    for (; z <= endz; z++) {
        memcpy(*dst, map_get_color_xy(map, xy, z), 3);
        (*dst)[3] = 0;

        *dst += 4;
    }
}

static MapCol get_surrounded(struct MapBuf * map, int xy) {
    MapCol colc = map_get_col_xy(map, xy);
    MapCol coll = -1;
    MapCol colr = -1;
    MapCol colu = -1;
    MapCol cold = -1;

    if (xy % MAP_X > 0)
        coll = map_get_col_xy(map, xy - 1);
    if (xy % MAP_X < MAP_X - 1)
        colr = map_get_col_xy(map, xy + 1);
    if (xy >= MAP_X)
        colu = map_get_col_xy(map, xy - MAP_X);
    if (xy < MAP_XY - MAP_X)
        cold = map_get_col_xy(map, xy + MAP_X);

    MapCol sidefilled = coll & colr & colu & cold;
    MapCol vertfilled = ((colc >> 1) | (1LLU << 63)) & (colc << 1);

    return sidefilled & vertfilled;
}

static unsigned char * write_vxl_column(unsigned char * dst, struct MapBuf * map, int xy) {
    MapCol surrounded = get_surrounded(map, xy);

    MapCol solid   = map_get_col_xy(map, xy);
    MapCol colored = solid & ~surrounded;
    MapCol filled  = solid &  surrounded;

    int next_air_start = 0;
    while (1) {
        int air_start = next_air_start;
        int top_start = map_col_ctz(solid, MAP_Z);

        colored |= bittbl[top_start];
        int filled_start = map_col_ctz(~colored, MAP_Z);
        int top_end = filled_start - 1;

        filled  |= bittbl[filled_start];
        int bottom_start = map_col_ctz(~filled, MAP_Z);

        if (likely(bottom_start == MAP_Z)) {
            dst[0] = 0;
            dst[1] = top_start;
            dst[2] = top_end;
            dst[3] = air_start;
            dst += 4;

            write_colors(&dst, map, xy, top_start, top_end);
            break;
        }

        colored |= bittbl[bottom_start];
        next_air_start = map_col_ctz(~colored, MAP_Z);

        int top_len    = filled_start   - top_start;
        int bottom_len = next_air_start - bottom_start;
        int bottom_end = next_air_start - 1;

        solid &= ~bittbl[next_air_start];

        dst[0] = 1 + top_len + bottom_len;
        dst[1] = top_start;
        dst[2] = top_end;
        dst[3] = air_start;
        dst += 4;

        write_colors(&dst, map, xy, top_start,    top_end);
        write_colors(&dst, map, xy, bottom_start, bottom_end);
    }

    return dst;
}

size_t vxl_write(void * dst, struct MapBuf * map, int xy, int cols) {
    unsigned char * cdst  = dst;
    unsigned char * start = cdst;

    int xy_end = xy + cols;

    map_affirm(xy >= 0);
    map_affirm(cols >= 0);
    map_affirm(xy + cols <= MAP_XY);

    for (; xy < xy_end; xy++)
        cdst = write_vxl_column(cdst, map, xy);

    return cdst - start;
}
