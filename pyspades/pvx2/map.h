#ifndef IzlnvsfFxjMISdrcNNnmUgIctrFClNxl
#define IzlnvsfFxjMISdrcNNnmUgIctrFClNxl

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

#define MapApi static inline __attribute__((always_inline))

/* Base data access */
MapApi long map_to_xy(int x, int y);

MapApi MapCol map_get_col_xy(struct MapBuf *, long      xy);
MapApi MapCol map_get_col   (struct MapBuf *, int x, int y);
MapApi void   map_set_col_xy(struct MapBuf *, long      xy, MapCol val);
MapApi void   map_set_col   (struct MapBuf *, int x, int y, MapCol val);

MapApi unsigned char *map_get_color_xy(struct MapBuf *, long      xy, int z);
MapApi unsigned char *map_get_color   (struct MapBuf *, int x, int y, int z);

MapApi void map_set_color_xy(
	struct MapBuf *, long      xy, int z, const void *colors, long len
);
MapApi void map_set_color(
	struct MapBuf *, int x, int y, int z, const void *colors, long len
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

#define affirm(x) __attribute__((assume(x)))

#define affirm_xy()    affirm(xy >= 0 && xy < MAP_XY)
#define affirm_x_y()   affirm(x >= 0 && x < MAP_X && y >= 0 && y < MAP_Y)
#define affirm_z()     affirm(z >= 0 && z < MAP_Z)
#define affirm_z_len() affirm(z >= 0 && len >= 0 && z + len <= MAP_Z)

#define affirm_xy_z()      do {affirm_xy();  affirm_z();} while (0)
#define affirm_x_y_z()     do {affirm_x_y(); affirm_z();} while (0)
#define affirm_xy_z_len()  do {affirm_xy();  affirm_z_len();} while (0)
#define affirm_x_y_z_len() do {affirm_x_y(); affirm_z_len();} while (0)

/* These don't invoke undefined behavior if y >= MAP_Z. */
#define col_shl(x, y) ((x) << ((y) & (MAP_Z - 1)))
#define col_shr(x, y) ((x) >> ((y) & (MAP_Z - 1)))

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
MapApi long map_to_xy(int x, int y)
{
	affirm_x_y();
	return y * MAP_X + x;
}

MapApi MapCol map_get_col_xy(struct MapBuf * map, long xy)
{
	affirm_xy();
	return map->solidbuf[xy];
}

MapApi MapCol map_get_col(struct MapBuf * map, int x, int y)
{
	affirm_x_y();
	return map_get_col_xy(map, map_to_xy(x, y));
}

MapApi void map_set_col_xy(struct MapBuf * map, long xy, MapCol val)
{
	affirm_xy();
	map->solidbuf[xy] = val;
}

MapApi void map_set_col(struct MapBuf * map, int x, int y, MapCol val)
{
	affirm_x_y();
	map_set_col_xy(map, map_to_xy(x, y), val);
}

MapApi unsigned char *map_get_color_xy(struct MapBuf * map, long xy, int z)
{
	affirm_xy_z();
	return map->colorbuf + (xy * MAP_Z + z) * 3;
}

MapApi unsigned char *map_get_color(struct MapBuf * map, int x, int y, int z)
{
	affirm_x_y_z();
	return map_get_color_xy(map, map_to_xy(x, y), z);
}

MapApi void map_set_color_xy(
	struct MapBuf * map, long xy, int z, const void *colors, long len
) {
	/* Affirming z seems to hurt more than it helps here. */
	affirm_xy();
	affirm(len >= 0 && len <= 64);
	memcpy(
		map->colorbuf + (xy * MAP_Z + z) * 3,
		colors,
		len * 3
	);
}

MapApi void map_set_color(
	struct MapBuf * map, int x, int y, int z, const void *colors, long len
) {
	affirm_x_y();
	affirm(len >= 0 && len <= 64);
	memcpy(
		map->colorbuf + (map_to_xy(x, y) * MAP_Z + z) * 3,
		colors,
		len * 3
	);
}

/*
 * Whole-map solidity manipulation
 */
MapApi void map_clear(struct MapBuf * map)
{
	memset(map->solidbuf, 0, sizeof(MapCol) * MAP_XY);
}

MapApi void map_copy(struct MapBuf * dst, const struct MapBuf * src)
{
	memcpy(dst->solidbuf, src->solidbuf, sizeof(dst->solidbuf));

	if (dst->colorbuf != NULL && src->colorbuf != NULL)
		memcpy(dst->colorbuf, src->colorbuf, 3 * MAP_XYZ);
}

/*
 * Column solidity manipulation
 */
MapApi void map_create_range_xy(struct MapBuf * map, long xy, int z, long len)
{
	affirm_xy_z_len();
	map_set_col_xy(map, xy,
		map_get_col_xy(map, xy) | col_shl(bittbl[len], z)
	);
}

MapApi void map_create_range(struct MapBuf * map, int x, int y, int z, long len)
{
	affirm_x_y_z_len();
	map_create_range_xy(map, map_to_xy(x, y), z, len);
}

MapApi void map_create_xy(struct MapBuf * map, long xy, int z)
{
	affirm_xy_z();
	map_create_range_xy(map, xy, z, 1);
}

MapApi void map_create(struct MapBuf * map, int x, int y, int z)
{
	affirm_x_y_z();
	map_create_range(map, x, y, z, 1);
}

MapApi void map_destroy_range_xy(struct MapBuf * map, long xy, int z, long len)
{
	affirm_xy_z_len();
	map_set_col_xy(map, xy,
		map_get_col_xy(map, xy) & ~col_shl(bittbl[len], z)
	);
}

MapApi void map_destroy_range(struct MapBuf * map, int x, int y, int z, long len)
{
	affirm_x_y_z_len();
	map_destroy_range_xy(map, map_to_xy(x, y), z, len);
}

MapApi void map_destroy_xy(struct MapBuf * map, long xy, int z)
{
	affirm_xy_z();
	map_destroy_range_xy(map, xy, z, 1);
}

MapApi void map_destroy(struct MapBuf * map, int x, int y, int z)
{
	affirm_x_y_z();
	map_destroy_range(map, x, y, z, 1);
}

/*
 * Column solidity querying
 */
MapApi MapCol map_is_solid_range_xy(struct MapBuf * map, long xy, int z, long len)
{
	affirm_xy_z_len();
	return col_shr(map_get_col_xy(map, xy), z) & bittbl[len];
}

MapApi MapCol map_is_solid_range(struct MapBuf * map, int x, int y, int z, long len)
{
	affirm_x_y_z_len();
	return map_is_solid_range_xy(map, map_to_xy(x, y), z, len);
}

MapApi int map_is_solid_xy(struct MapBuf * map, long xy, int z)
{
	affirm_xy_z();
	return map_is_solid_range_xy(map, xy, z, 1);
}

MapApi int map_is_solid(struct MapBuf * map, int x, int y, int z)
{
	affirm_x_y_z();
	return map_is_solid_range(map, x, y, z, 1);
}

MapApi int map_count_solids_xy(struct MapBuf * map, long xy, int z)
{
	affirm_xy_z();
	return __builtin_ctzg(~map_get_col_xy(map, xy) >> z, MAP_Z);
}

MapApi int map_count_solids(struct MapBuf * map, int x, int y, int z)
{
	affirm_x_y_z();
	return map_count_solids_xy(map, map_to_xy(x, y), z);
}

MapApi int map_count_nonsolids_xy(struct MapBuf * map, long xy, int z)
{
	affirm_xy_z();
	return __builtin_ctzg(map_get_col_xy(map, xy) >> z, MAP_Z - z);
}

MapApi int map_count_nonsolids(struct MapBuf * map, int x, int y, int z)
{
	affirm_x_y_z();
	return map_count_nonsolids_xy(map, map_to_xy(x, y), z);
}

MapApi int map_get_floor_xy(struct MapBuf * map, long xy, int z)
{
	affirm_xy_z();
	return z + map_count_nonsolids_xy(map, xy, z);
}

MapApi int map_get_floor(struct MapBuf * map, int x, int y, int z)
{
	affirm_x_y_z();
	return map_get_floor_xy(map, map_to_xy(x, y), z);
}

MapApi int map_get_top_xy(struct MapBuf * map, long xy)
{
	affirm_xy();
	return __builtin_ctzg(map_get_col_xy(map, xy), MAP_Z);
}

MapApi int map_get_top(struct MapBuf * map, int x, int y)
{
	affirm_x_y();
	return map_get_top_xy(map, map_to_xy(x, y));
}

#undef col_shr
#undef col_shl

#undef affirm_x_y_z_len
#undef affirm_xy_z_len
#undef affirm_x_y_z
#undef affirm_xy_z

#undef affirm_z_len
#undef affirm_z
#undef affirm_x_y
#undef affirm_xy

#undef affirm

#endif
