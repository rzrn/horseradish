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

typedef struct CullBuf *CullState;

int  map_cull_alloc(CullState *cull);
void map_cull_free (CullState *cull);

void map_cull_end(CullState cull);

void map_cull_xy(Map map, CullState cull, long xy,      int z);
void map_cull   (Map map, CullState cull, int x, int y, int z);

#endif
