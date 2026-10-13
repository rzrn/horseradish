#ifndef SFkYRQFpdRlBXBysSUPMtnQDGlMXXKia
#define SFkYRQFpdRlBXBysSUPMtnQDGlMXXKia

#include "map.h"

struct CullBuf {
	int grnd_ptr;

	MapCol traversed[MAP_XY];
	MapCol grounded[MAP_XY];

	int traversed_xy[MAP_XY];
	int grounded_xy[MAP_XY];
	int to_visit_xy[MAP_XY];
	unsigned char visiting[MAP_XY];
};

void map_cull_end(struct CullBuf *);

void map_cull_xy(struct MapBuf *, struct CullBuf *, long xy,      int z);
void map_cull   (struct MapBuf *, struct CullBuf *, int x, int y, int z);

#endif
