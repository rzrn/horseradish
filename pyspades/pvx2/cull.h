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

#ifndef PVX2_CULL_H
#define PVX2_CULL_H

#include "map.h"

struct CullBuf {
    int grnd_ptr;

    MapCol traversed[MAP_XY];
    MapCol grounded[MAP_XY];

    int traversed_xy[MAP_XY];
    int grounded_xy[MAP_XY];
    int to_visit_xy[MAP_XY];
    unsigned char visiting[MAP_XY];
};

void map_cull_end(struct CullBuf *);

int map_cull_xy(struct MapBuf *, struct CullBuf *, long xy,      int z);
int map_cull   (struct MapBuf *, struct CullBuf *, int x, int y, int z);

#endif
