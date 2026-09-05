#include "write.h"
#include <string.h>

#define bsf(x) __builtin_ctzll(x)

#define affirm(x)   __attribute__((assume(x)))
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

typedef unsigned long long u64;

static void write_color(
	unsigned char *color, const unsigned char *buf, int xy, int z
) {
	memcpy(color, buf + (xy * MAP_Z + z) * 3, 3);
	color[3] = 0;
}

static u64 get_surrounded(const u64 *solidbuf, int xy)
{
	u64 colc = solidbuf[xy];
	u64 coll = -1;
	u64 colr = -1;
	u64 colu = -1;
	u64 cold = -1;

	if (xy % MAP_X > 0)
		coll = solidbuf[xy-1];
	if (xy % MAP_X < MAP_X - 1)
		colr = solidbuf[xy+1];
	if (xy >= MAP_X)
		colu = solidbuf[xy-MAP_X];
	if (xy < (MAP_X * MAP_Y) - MAP_X)
		cold = solidbuf[xy+MAP_X];

	u64 sidefilled = coll & colr & colu & cold;
	u64 vertfilled = ((colc >> 1) | (1LLU << 63)) & (colc << 1);

	return sidefilled & vertfilled;
}

static unsigned char *write_vxl_column(
	unsigned char *dst, const struct MapBuf *map, int xy
) {
	unsigned char *colorbuf = map->colorbuf;
	const u64 *solidbuf64 = (u64 *)map->solidbuf;

	u64 surrounded = get_surrounded(solidbuf64, xy);

	u64 solid   = solidbuf64[xy];
	u64 colored = solid & ~surrounded;
	u64 filled  = solid &  surrounded;

#define shift(x) do { \
	int sh = x; \
	solid >>= sh; colored >>= sh; filled >>= sh; \
} while (0)

	int next_air_start = 0;
	while (1) {
		int cnt;
		int z;

		int span_len = 1;
		int air_start = next_air_start;

		if (unlikely(solid == 0)) {
			/* bsf doesn't play well with all zeroes */
			dst[0] = 0;
			dst[1] = 64;
			dst[2] = 63;
			dst[3] = air_start;
			dst += 4;

			break;
		}

		shift((cnt = bsf(solid)));
		int top_start = air_start + cnt;
		int top_end;

		/* Nonsolid water can cause all voxels in a
		 * column to be colored, which bsf still hates.
		 */
		if (colored != (u64)-1) {
			shift((cnt = bsf(~colored)));
			top_end = top_start + cnt - 1;
			span_len += cnt;
		} else
			top_end = MAP_Z - 1;

		shift((cnt = bsf(~filled)));
		int bottom_start = top_end + cnt + 1;

		if (likely(bottom_start == MAP_Z)) {
			dst[0] = 0;
			dst[1] = top_start;
			dst[2] = top_end;
			dst[3] = air_start;
			dst += 4;

			for (z = top_start; z <= top_end; z++) {
				write_color(dst, colorbuf, xy, z);
				dst += 4;
			}

			break;
		}

		shift((cnt = bsf(~colored)));
		next_air_start = bottom_start + cnt;
		span_len += cnt;

		dst[0] = span_len;
		dst[1] = top_start;
		dst[2] = top_end;
		dst[3] = air_start;
		dst += 4;

		for (z = top_start; z <= top_end; z++) {
			write_color(dst, colorbuf, xy, z);
			dst += 4;
		}

		for (z = bottom_start; z < next_air_start; z++) {
			write_color(dst, colorbuf, xy, z);
			dst += 4;
		}
	}

	return dst;
}

size_t vxl_write(void *dst, const struct MapBuf *map, int xy, int cols)
{
	unsigned char *cdst = dst;
	unsigned char *start = cdst;
	int xy_end = xy + cols;

	affirm(xy >= 0);
	affirm(cols >= 0);
	affirm(xy + cols <= MAP_X * MAP_Y);

	for (; xy < xy_end; xy++)
		cdst = write_vxl_column(cdst, map, xy);

	return cdst - start;
}
