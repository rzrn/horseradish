#ifndef SFkYRQFpdRlBXBysSUPMtnQDGlMXXKia
#define SFkYRQFpdRlBXBysSUPMtnQDGlMXXKia

#include "map.h"

struct CullBuf {
	int fdptr;

	MapCol traversed[MAP_XY];
	MapCol grounded[MAP_XY];

	int now_found[MAP_XY];
	int all_found[MAP_XY];
	int to_visit[MAP_XY];
	unsigned char visiting[MAP_XY];
};

typedef struct CullBuf *CullState;

int  map_cull_alloc(CullState *cull);
void map_cull_end(CullState cull);
void map_cull_xy(Map map, CullState cull, int xy,       int z);
void map_cull   (Map map, CullState cull, int x, int y, int z);

#endif
