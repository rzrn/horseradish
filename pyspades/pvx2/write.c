#include "write.h"
#include <string.h>

#define tzcnt(x) __builtin_ctzll(x)

typedef unsigned long long u64;

static void get_color(
	unsigned char *color, int xy, int z, const unsigned char *buf
) {
	memcpy(color, buf + (xy * VXL_Z + z) * 3, 3);
}

static unsigned char *write_vxl_column(unsigned char *dst, const struct VXLMapBuf *map, int xy)
{
	u64 *buf64 = (u64 *)map->solidbuf;

	unsigned char *colorbuf = map->colorbuf;

	u64 colc = buf64[xy];
	u64 coll = -1;
	u64 colr = -1;
	u64 colu = -1;
	u64 cold = -1;

	if (xy % VXL_X > 0)
		coll = buf64[xy-1];
	if (xy % VXL_X < VXL_X - 1)
		colr = buf64[xy+1];
	if (xy >= VXL_X)
		colu = buf64[xy-VXL_X];
	if (xy < (VXL_X * VXL_Y) - VXL_X)
		cold = buf64[xy+VXL_X];

	u64 sidefilled = coll & colr & colu & cold;
	u64 vertfilled = ((colc >> 1) | (1LLU << 63)) & (colc << 1);

	u64 solid   = colc;
	u64 colored = solid & ~(sidefilled & vertfilled);
	u64 filled  = solid & ~colored;

#define shift(x) do { \
	solid >>= x; colored >>= x; filled >>= x; \
} while (0)

	int next_air_start = 0;
	while (1) {
		int air_start = next_air_start;
		int span_len = 1;
		int cnt;
		int z;

		if (solid == 0) {
			/* tzcnt doesn't play well with all zeroes */
			dst[0] = 0;
			dst[1] = 64;
			dst[2] = 63;
			dst[3] = air_start;
			dst += 4;

			break;
		}

		cnt = tzcnt(solid);
		shift(cnt);
		int top_start = air_start + cnt;

		cnt = tzcnt(~colored);
		shift(cnt);
		int top_end = top_start + cnt - 1;
		span_len += cnt;

		cnt = tzcnt(~filled);
		shift(cnt);
		int bottom_start = top_end + cnt + 1;

		if (bottom_start == VXL_Z) {
			dst[0] = 0;
			dst[1] = top_start;
			dst[2] = top_end;
			dst[3] = air_start;
			dst += 4;

			for (z = top_start; z <= top_end; z++) {
				get_color(dst, xy, z, colorbuf);
				dst += 4;
			}

			break;
		}

		cnt = tzcnt(~colored);
		shift(cnt);
		next_air_start = bottom_start + cnt;
		span_len += cnt;

		dst[0] = span_len;
		dst[1] = top_start;
		dst[2] = top_end;
		dst[3] = air_start;
		dst += 4;

		for (z = top_start; z <= top_end; z++) {
			get_color(dst, xy, z, colorbuf);
			dst += 4;
		}

		for (z = bottom_start; z < next_air_start; z++) {
			get_color(dst, xy, z, colorbuf);
			dst += 4;
		}
	}

	return dst;
}

size_t vxl_write(void *dst, const struct VXLMapBuf *map, int xy, int cols)
{
	unsigned char *cdst = dst;
	unsigned char *start = cdst;
	int xy_end = xy + cols;

	for (; xy < xy_end; xy++)
		cdst = write_vxl_column(cdst, map, xy);

	return cdst - start;
}
