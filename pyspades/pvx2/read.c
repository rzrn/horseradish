#include "read.h"
#include <string.h>

#define check(condition) do {if (!(condition)) return -1;} while (0)

static int read_vxl_span_hdr(struct VXLState *state, const unsigned char **buf)
{
    int span_len  = (*buf)[0];
    int top_start = (*buf)[1];
    int top_end   = (*buf)[2];

    int top_len = (top_end - top_start) + 1;
    int last_span = span_len == 0;

    check(top_end < MAP_Z);
    check(span_len <= (MAP_Z + 1));

    check(top_start <= top_end + 1);

    if (last_span)
        span_len = top_len; /* length of next span hdr excluded */
    else {
        int bottom_len = (span_len - 1) - top_len;
        check(bottom_len >= 0);

        state->bottom_len = bottom_len;
    }

    state->top_start = top_start;
    state->top_end   = top_end;
    state->last_span = last_span;

    state->span_nbytes = span_len * 4;

    *buf += 4;
    return 0;
}

#define r04(x, y, z)     x, y, z,      x, y, z,      x, y, z,      x, y, z
#define r16(x, y, z) r04(x, y, z), r04(x, y, z), r04(x, y, z), r04(x, y, z)
#define r64(x, y, z) r16(x, y, z), r16(x, y, z), r16(x, y, z), r16(x, y, z)
static const unsigned char dirt_color[64 * 3] = {r64(40, 64, 103)};

static int read_vxl_span_body(struct VXLState *state, const unsigned char **buf)
{
    struct MapBuf * map = state->map;

    int xy         = state->xy;
    int z          = state->top_start;
    int top_end    = state->top_end;
    int bottom_len = state->bottom_len;
    int last_span  = state->last_span;

    map_create_range_xy(map, xy, z, top_end + 1 - z);
    for (; z <= top_end; z++) {
        map_set_color_xy(map, xy, z, *buf, 1);
        *buf += 4;
    }

    if (last_span) {
        map_create_range_xy(map, xy, z, MAP_Z - z);
        map_set_color_xy(map, xy, z, dirt_color, MAP_Z - z);

        return 1;
    }

    int next_air_start = (*buf + bottom_len * 4)[3];
    int bottom_start = next_air_start - bottom_len;

    check(next_air_start <= MAP_Z);
    check(z <= bottom_start);

    map_create_range_xy(map, xy, z, next_air_start - z);
    map_set_color_xy(map, xy, z, dirt_color, bottom_start - z);

    z = bottom_start;

    for (; z < next_air_start; z++) {
        map_set_color_xy(map, xy, z, *buf, 1);
        *buf += 4;
    }

    return read_vxl_span_hdr(state, buf);
}

#define REMAINING (end - csrc)
int vxl_read(struct MapBuf * map, const void *src, size_t nbytes)
{
    const unsigned char *csrc = src;
    const unsigned char *end = csrc + nbytes;

    struct VXLState state;
    int xy;

    map_clear(map);
    state.map = map;

    for (xy = 0; xy < MAP_XY; xy++) {
        state.xy = xy;

        check(REMAINING >= 4);
        check(read_vxl_span_hdr(&state, &csrc) == 0);

        while (1) {
            int status;

            check(REMAINING >= state.span_nbytes);
            status = read_vxl_span_body(&state, &csrc);

            if (status == 1)
                break;

            check(status == 0);
        }
    }

    return 1;
}

void vxl_stream_read_open(struct VXLStream *stream, struct MapBuf * map)
{
    map_clear(map);
    stream->state.map         = map;

    stream->state.xy          = 0;
    stream->state.span_nbytes = 4;
    stream->part              = 0;
    stream->span_buf_nbytes   = 0;
}

int vxl_stream_read(struct VXLStream *stream, const void *src, size_t nbytes)
{
    const unsigned char *csrc = src;
    const unsigned char *end = csrc + nbytes;

    int status;

    unsigned char *ptr  = stream->span_buf + stream->span_buf_nbytes;
    int to_cpy = stream->state.span_nbytes - stream->span_buf_nbytes;
    const unsigned char *inptr = stream->span_buf;

    if (REMAINING < to_cpy) {
        memcpy(ptr, csrc, REMAINING);
        stream->span_buf_nbytes += REMAINING;
        return 0;
    }

    memcpy(ptr, csrc, to_cpy);
    csrc += to_cpy;

    if (stream->part == 0) {
        check(read_vxl_span_hdr(&stream->state, &inptr) == 0);
        goto post_hdr;
    }

    status = read_vxl_span_body(&stream->state, &inptr);
    goto post_body;

    for (; stream->state.xy < MAP_XY; stream->state.xy++) {
        if (REMAINING < 4) {
            stream->part = 0;
            stream->state.span_nbytes = 4;

            stream->span_buf_nbytes = REMAINING;
            memcpy(stream->span_buf, csrc, REMAINING);
            return 0;
        }

        check(read_vxl_span_hdr(&stream->state, &csrc) == 0);
        post_hdr:

        while (1) {
            if (REMAINING < stream->state.span_nbytes) {
                stream->part = 1;

                stream->span_buf_nbytes = REMAINING;
                memcpy(stream->span_buf, csrc, REMAINING);
                return 0;
            }

            status = read_vxl_span_body(&stream->state, &csrc);
            post_body:

            if (status == 1)
                break;

            check(status == 0);
        }
    }

    return 1;
}
