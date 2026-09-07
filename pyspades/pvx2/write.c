#include "write.h"
#include <string.h>

#define tzcnt(x) __builtin_ctzg(x, MAP_Z)

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

static unsigned char *write_vxl_column(unsigned char *dst, Map map, int xy)
{
	MapCol surrounded = get_surrounded(map, xy);

	MapCol solid   = map_get_col_xy(map, xy);
	MapCol colored = solid & ~surrounded;
	MapCol filled  = solid &  surrounded;

	int next_air_start = 0;
	while (1) {
		int z;

		int air_start = next_air_start;
		int top_start = tzcnt(colored);

		colored |= bittbl[top_start];
		int filled_start = tzcnt(~colored);
		int top_end = filled_start - 1;

		filled  |= bittbl[filled_start];
		int bottom_start = tzcnt(~filled);

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

		colored |= bittbl[bottom_start];
		next_air_start = tzcnt(~colored);

		int top_len    = filled_start   - top_start;
		int bottom_len = next_air_start - bottom_start;

		MapCol mask = bittbl[next_air_start];
		colored &= ~mask;

		dst[0] = 1 + top_len + bottom_len;
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
