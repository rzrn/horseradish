#include "write.h"
#include <string.h>

#define bsf(x) __builtin_ctzll(x)

#define affirm(x)   __attribute__((assume(x)))
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

static void write_color(unsigned char *dst, Map map, int xy, int z)
{
	memcpy(dst, map_get_color_xy(map, xy, z), 3);
	dst[3] = 0;
}

static MapCol get_surrounded(Map map, int xy)
{
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

#define shift(x) do { \
	int sh = x; \
	solid >>= sh; colored >>= sh; filled >>= sh; \
} while (0)

static unsigned char *write_vxl_column(unsigned char *dst, Map map, int xy)
{
	MapCol surrounded = get_surrounded(map, xy);

	MapCol solid   = map_get_col_xy(map, xy);
	MapCol colored = solid & ~surrounded;
	MapCol filled  = solid &  surrounded;

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
		if (colored != (MapCol)-1) {
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
				write_color(dst, map, xy, z);
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
			write_color(dst, map, xy, z);
			dst += 4;
		}

		for (z = bottom_start; z < next_air_start; z++) {
			write_color(dst, map, xy, z);
			dst += 4;
		}
	}

	return dst;
}

size_t vxl_write(void *dst, Map map, int xy, int cols)
{
	unsigned char *cdst = dst;
	unsigned char *start = cdst;
	int xy_end = xy + cols;

	affirm(xy >= 0);
	affirm(cols >= 0);
	affirm(xy + cols <= MAP_XY);

	for (; xy < xy_end; xy++)
		cdst = write_vxl_column(cdst, map, xy);

	return cdst - start;
}
