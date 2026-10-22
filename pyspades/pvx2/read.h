/*
    Copyright © 2026 notaburner <davidco7777@protonmail.com>

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

#ifndef PVX2_READ_H
#define PVX2_READ_H

#include "map.h"
#include <stddef.h>

/* Non-streaming-exclusive VXL decode state */
struct VXLState {
    int xy;

    int top_start;
    int top_end;
    int bottom_len;
    int last_span;

    struct MapBuf * map;

    /* This'll include the next span hdr in the length
     * if not the last span.
     */
    int span_nbytes;
};

struct VXLStream {
    struct VXLState state;

    int part;

    /* This'll include the next span hdr at the end
     * of it if not the last span, hence the + 1.
     */
    int span_buf_nbytes;
    unsigned char span_buf[(MAP_Z + 1) * 4];
};

int vxl_read(struct MapBuf *, const void *src, size_t nbytes);

void vxl_stream_read_open(struct VXLStream *stream, struct MapBuf *);
int vxl_stream_read(struct VXLStream *stream, const void *src, size_t nbytes);

#endif
