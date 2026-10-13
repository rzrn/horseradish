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
	unsigned char *colorbuf;
	MapCol         solidbuf[MAP_XY];
	unsigned char  colorbuf_mem[MAP_XYZ * 3];
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

#include "map.c"

#endif
