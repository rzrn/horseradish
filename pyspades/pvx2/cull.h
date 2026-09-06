#ifndef SFkYRQFpdRlBXBysSUPMtnQDGlMXXKia
#define SFkYRQFpdRlBXBysSUPMtnQDGlMXXKia

#include "map.h"

struct CullBuf {
	int fdptr;

	MapCol *traversed;
	MapCol *grounded;

	int *now_found;
	int *all_found;
	int *to_visit;
	unsigned char *visiting;
};

int map_cull_alloc(struct CullBuf *cull);
void map_cull_end(struct CullBuf *cull);
void map_cull_xy(Map map, struct CullBuf *cull, int xy, int z);
void map_cull(Map map, struct CullBuf *cull, int x, int y, int z);

#endif
