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

	/* TODO: test with nonsolid waters */
	check(top_end < VXL_Z);
	check(span_len <= (VXL_Z + 1));

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

/* TODO: probably just consistently use a uint64_t instead of uint8_t for
 * solid buf
 */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ && VXL_Z == 64
/* left shift by >= bitwidth is UB */
#define lshift64(x, y) ((y) >= 64 ? 0 : ((unsigned long long)(x) << (y)))

/* Lookup table is a little faster than the comparison for defined left shift */
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
	-1
};

static void make_solids(unsigned char *buf, int xy, int z, int len)
{
	unsigned long long *buf64 = (unsigned long long *)buf;

	buf64[xy] |= lshift64(bittbl[len], z);
}
#else
static void make_solid(unsigned char *buf, int xy, int z)
{
	int off = xy * VXL_Z + z;
	buf[off / 8] |= 1 << (off & 7);
}

static void make_solids(unsigned char *buf, int xy, int z, int len)
{
	int dest = z + len;

	for (; z < dest; z++)
		make_solid(buf, xy, z);
}
#endif

static void copy_color(
	unsigned char *buf, int xy, int z, const unsigned char *color
) {
	memcpy(buf + (xy * VXL_Z + z) * 3, color, 3);
}

static void copy_colors(
	unsigned char *buf, int xy, int zstart,
	const unsigned char *colors, int count
) {
	memcpy(buf + (xy * VXL_Z + zstart) * 3, colors, count * 3);
}

#define rep04(x, y, z) x, y, z,   x, y, z,   x, y, z,   x, y, z
#define rep16(x, y, z) rep04(x, y, z), rep04(x, y, z), rep04(x, y, z), rep04(x, y, z)
#define rep64(x, y, z) rep16(x, y, z), rep16(x, y, z), rep16(x, y, z), rep16(x, y, z)
static const unsigned char dirt_color[64 * 3] = {rep64(40, 64, 103)};

static int read_vxl_span_body(struct VXLState *state, const unsigned char **buf)
{
	struct VXLMapBuf map = state->map;

	int xy         = state->xy;

	int z          = state->top_start;
	int top_end    = state->top_end;
	int bottom_len = state->bottom_len;
	int last_span  = state->last_span;

	make_solids(map.solidbuf, xy, z, top_end + 1 - z);
	for (; z <= top_end; z++) {
		copy_color(map.colorbuf, xy, z, *buf);
		*buf += 4;
	}

	if (last_span) {
		make_solids(map.solidbuf, xy, z, VXL_Z - z);
		copy_colors(map.colorbuf, xy, z, dirt_color, VXL_Z - z);

		return 1;
	}

	int next_air_start = (*buf + bottom_len * 4)[3];
	int bottom_start = next_air_start - bottom_len;

	check(next_air_start < VXL_Z);
	check(z <= bottom_start);

	make_solids(map.solidbuf, xy, z, next_air_start - z);
	copy_colors(map.colorbuf, xy, z, dirt_color, bottom_start - z);

	z = bottom_start;

	for (; z < next_air_start; z++) {
		copy_color(map.colorbuf, xy, z, *buf);
		*buf += 4;
	}

	return read_vxl_span_hdr(state, buf);
}

#define REMAINING (end - csrc)
int vxl_read(const struct VXLMapBuf *map, const void *src, size_t nbytes)
{
	const unsigned char *csrc = src;
	const unsigned char *end = csrc + nbytes;

	struct VXLState state;
	int xy;

	memset(map->solidbuf, 0, VXL_SOLIDBUF_SIZ);
	state.map = *map;

	for (xy = 0; xy < (VXL_X * VXL_Y); xy++) {
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

void vxl_stream_read_open(struct VXLStream *stream, const struct VXLMapBuf *map)
{
	memset(map->solidbuf, 0, VXL_SOLIDBUF_SIZ);
	stream->state.map = *map;

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

	for (; stream->state.xy < (VXL_X * VXL_Y); stream->state.xy++) {
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
