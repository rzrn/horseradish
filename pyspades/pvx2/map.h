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

typedef struct MapBuf *Map;

#define MapApi static inline __attribute__((always_inline))

/* Allocation */
MapApi int  map_alloc(Map *map);
MapApi void map_free (Map *map);

/* Base data access */
MapApi long map_to_xy(int x, int y);

MapApi MapCol map_get_col_xy(Map map, long      xy);
MapApi MapCol map_get_col   (Map map, int x, int y);
MapApi void   map_set_col_xy(Map map, long      xy, MapCol val);
MapApi void   map_set_col   (Map map, int x, int y, MapCol val);

MapApi unsigned char *map_get_color_xy(Map map, long      xy, int z);
MapApi unsigned char *map_get_color   (Map map, int x, int y, int z);

MapApi void map_set_color_xy(
	Map map, long      xy, int z, const void *colors, long len
);
MapApi void map_set_color(
	Map map, int x, int y, int z, const void *colors, long len
);

/* Whole-map solidity manipulation */
MapApi void map_clear(Map map);

/* Column solidity manipulation */
MapApi void map_create_range_xy (Map map, long      xy, int z, long len);
MapApi void map_create_range    (Map map, int x, int y, int z, long len);
MapApi void map_create_xy       (Map map, long      xy, int z);
MapApi void map_create          (Map map, int x, int y, int z);

MapApi void map_destroy_range_xy(Map map, long      xy, int z, long len);
MapApi void map_destroy_range   (Map map, int x, int y, int z, long len);
MapApi void map_destroy_xy      (Map map, long      xy, int z);
MapApi void map_destroy         (Map map, int x, int y, int z);

/* Column solidity querying */
MapApi MapCol map_is_solid_range_xy (Map map, long      xy, int z, long len);
MapApi MapCol map_is_solid_range    (Map map, int x, int y, int z, long len);
MapApi int    map_is_solid_xy       (Map map, long      xy, int z);
MapApi int    map_is_solid          (Map map, int x, int y, int z);

MapApi int    map_count_solids_xy   (Map map, long      xy, int z);
MapApi int    map_count_solids      (Map map, int x, int y, int z);
MapApi int    map_count_nonsolids_xy(Map map, long      xy, int z);
MapApi int    map_count_nonsolids   (Map map, int x, int y, int z);

MapApi int map_get_floor_xy(Map map, long      xy, int z);
MapApi int map_get_floor   (Map map, int x, int y, int z);
MapApi int map_get_top_xy  (Map map, long      xy);
MapApi int map_get_top     (Map map, int x, int y);

#include "map.c"

#endif
