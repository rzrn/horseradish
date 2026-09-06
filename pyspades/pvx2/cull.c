#include "cull.h"
#include <stdlib.h>

#define lzcnt(x) __builtin_clzg(x, MAP_Z)
#define tzcnt(x) __builtin_ctzg(x, MAP_Z)

#define ror(x, y) (((x) >> (y)) | ((x) << (MAP_Z - (y))))

#define affirm(x)   __attribute__((assume(x)))
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

int map_cull_alloc(CullState *cull)
{
	CullState buf = calloc(1, sizeof(**cull));
	if (buf == NULL)
		return -1;

	*cull = buf;
	return 0;
}

void map_cull_end(CullState cull)
{
	int     fdptr     = cull->fdptr;
	int    *all_found = cull->all_found;
	MapCol *grounded  = cull->grounded;

	affirm(fdptr >= 0 && fdptr < MAP_XY);

	while (fdptr-- > 0)
		grounded[all_found[fdptr]] = 0;

	cull->fdptr = 0;
}

static void unwind_floating(Map map, CullState cull, int nfdptr)
{
	int    *now_found = cull->now_found;
	MapCol *traversed = cull->traversed;

	while (nfdptr-- != 0) {
		int xy = now_found[nfdptr];

		MapCol col = map_get_col_xy(map, xy);
		map_set_col_xy(map, xy, col & ~traversed[xy]);

		traversed[xy] = 0;
	}
}

static void unwind_grounded(CullState cull, int nfdptr)
{
	int    *now_found = cull->now_found;
	MapCol *traversed = cull->traversed;
	MapCol *grounded  = cull->grounded;

	unsigned char *visiting = cull->visiting;

	while (nfdptr-- != 0) {
		int xy = now_found[nfdptr];

		visiting[xy/8] = 0;

		grounded[xy] |= traversed[xy];
		traversed[xy] = 0;
	}
}

static MapCol isolate_col(MapCol col, int z)
{
	affirm(z >= 0 && z < MAP_Z);

	if (col == (MapCol)-1)
		return MAP_Z;

	col = ror(~col, z);

	int bits_lt_z = lzcnt(col);
	int bits_ge_z = tzcnt(col);

	int top = z - bits_lt_z;
	int len = bits_lt_z + bits_ge_z;

	return ((1LLU << len) - 1) << top;
}

static MapCol isolate_neighbors(MapCol adjacent, MapCol test)
{
	MapCol checked  = 0;

	while (1) {
		int next_col = tzcnt(adjacent);

		if (next_col == 64)
			break;

		checked  |= isolate_col(test, next_col);
		adjacent &= ~checked;
	}

	return checked;
}

static int visitf(
	CullState cull,
	Map map,
	MapCol col,
	int xy,
	int *tvptr,
	int *fdptr,
	int *nfdptr
) {
	int    *now_found = cull->now_found;
	int    *all_found = cull->all_found;
	int    *to_visit  = cull->to_visit;
	MapCol *traversed = cull->traversed;
	MapCol *grounded  = cull->grounded;

	unsigned char *visiting = cull->visiting;

	MapCol travr = traversed[xy];
	MapCol grnd  = grounded[xy];

	MapCol solid = map_get_col_xy(map, xy);
	MapCol untrv = solid & ~travr;

	/* TODO: ambiguous col term */
	MapCol new_adjacent = col & untrv;

	if (!new_adjacent)
		return 0;

	if (col & grnd)
		return 1;

	MapCol newtv = travr | isolate_neighbors(new_adjacent, solid);
	traversed[xy] = newtv;

	if (travr == 0) {
		now_found[(*nfdptr)++] = xy;
		if (grnd == 0)
			all_found[(*fdptr)++] = xy;
	}

	if (newtv & (1LLU << (MAP_Z - 2)))
		return 1;

	if (visiting[xy/8] & (1 << (xy & 7)))
		return 0;

	visiting[xy/8] |= 1 << (xy & 7);
	to_visit[(*tvptr)++] = xy;

	return 0;
}

#define visit(test_xy) do { \
	if (visitf( \
		cull, \
		map, \
		col, \
		test_xy, \
		&tvptr, \
		&fdptr, \
		&nfdptr \
	)) goto unwind; \
} while (0)

void map_cull_xy(Map map, CullState cull, int xy, int z)
{
	int fdptr  = cull->fdptr;
	int nfdptr = 0;
	int tvptr  = 0;

	MapCol col = map_get_col_xy(map, xy);
	if (!(col & (1LLU << z)))
		return;

	col = isolate_col(col, z);
	cull->traversed[xy] = col;
	cull->now_found[nfdptr++] = xy;
	cull->all_found[ fdptr++] = xy;

	/* TODO: can water fall on buggyspades? */
	if (col & (1LLU << (MAP_Z - 2)))
		goto unwind;

	while (1) {
		if (xy % MAP_X > 0)
			visit(xy - 1);
		if (xy % MAP_X < MAP_X - 1)
			visit(xy + 1);
		if (xy >= MAP_X)
			visit(xy - MAP_X);
		if (xy < MAP_XY - MAP_X)
			visit(xy + MAP_X);

		if (tvptr == 0)
			break;

		xy  = cull->to_visit[--tvptr];
		col = cull->traversed[xy];

		cull->visiting[xy/8] &= ~(1 << (xy & 7));
	}

	cull->fdptr = fdptr;
	return unwind_floating(map, cull, nfdptr);

	unwind:
	cull->fdptr = fdptr;
	return unwind_grounded(cull, nfdptr);
}

void map_cull(Map map, CullState cull, int x, int y, int z)
{
	map_cull_xy(map, cull, map_to_xy(x, y), z);
}
