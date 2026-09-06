#include "cull.h"
#include <stdlib.h>

#define bsr(x)   __builtin_clzll(x)
#define bsf(x)   __builtin_ctzll(x)

#define lzcnt(x) __builtin_clzg(x, MAP_Z)
#define tzcnt(x) __builtin_ctzg(x, MAP_Z)

#define ror(x, y) (((x) >> (y)) | ((x) << (MAP_Z - (y))))

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
	int     grnd_ptr    = cull->grnd_ptr;
	int    *grounded_xy = cull->grounded_xy;
	MapCol *grounded    = cull->grounded;

	while (grnd_ptr-- > 0)
		grounded[grounded_xy[grnd_ptr]] = 0;

	cull->grnd_ptr = 0;
}

static void unwind_floating(Map map, CullState cull, int trvr_ptr)
{
	int    *traversed_xy = cull->traversed_xy;
	MapCol *traversed    = cull->traversed;

	while (trvr_ptr-- != 0) {
		int xy = traversed_xy[trvr_ptr];

		MapCol col = map_get_col_xy(map, xy);
		map_set_col_xy(map, xy, col & ~traversed[xy]);

		traversed[xy] = 0;
	}
}

static void unwind_grounded(CullState cull, int trvr_ptr)
{
	int    *traversed_xy = cull->traversed_xy;
	MapCol *traversed    = cull->traversed;
	MapCol *grounded     = cull->grounded;

	unsigned char *visiting = cull->visiting;

	while (trvr_ptr-- != 0) {
		int xy = traversed_xy[trvr_ptr];

		visiting[xy/8] = 0;

		grounded[xy] |= traversed[xy];
		traversed[xy] = 0;
	}
}

static MapCol isolate_col(MapCol col, int z)
{
	if (col == (MapCol)-1)
		return MAP_Z;

	col = ror(~col, z);

	int bits_lt_z = bsr(col);
	int bits_ge_z = bsf(col);

	int top = z - bits_lt_z;
	int len = bits_lt_z + bits_ge_z;

	return ((1LLU << len) - 1) << top;
}

static MapCol isolate_neighbors(MapCol adjacent, MapCol test)
{
	MapCol checked  = 0;

	while (1) {
		int next_col = tzcnt(adjacent);

		if (next_col == MAP_Z)
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
	int *tovi_ptr,
	int *trvr_ptr,
	int *grnd_ptr
) {
	int    *traversed_xy = cull->traversed_xy;
	int    *grounded_xy  = cull->grounded_xy;
	int    *to_visit_xy  = cull->to_visit_xy;
	MapCol *traversed    = cull->traversed;
	MapCol *grounded     = cull->grounded;

	unsigned char *visiting = cull->visiting;

	MapCol travr = traversed[xy];
	MapCol grnd  = grounded[xy];
	MapCol solid = map_get_col_xy(map, xy);

	MapCol untravr = solid & ~travr;
	MapCol untravr_adjacent = untravr & col;

	if (!untravr_adjacent)
		return 0;

	if (col & grnd)
		return 1;

	MapCol new_travr = travr | isolate_neighbors(untravr_adjacent, solid);
	traversed[xy] = new_travr;

	if (travr == 0) {
		traversed_xy[(*trvr_ptr)++] = xy;
		if (grnd == 0)
			grounded_xy[(*grnd_ptr)++] = xy;
	}

	if (new_travr & (1LLU << (MAP_Z - 2)))
		return 1;

	if (visiting[xy/8] & (1 << (xy & 7)))
		return 0;

	visiting[xy/8] |= 1 << (xy & 7);
	to_visit_xy[(*tovi_ptr)++] = xy;

	return 0;
}

#define visit(test_xy) do { \
	if (visitf( \
		cull, \
		map, \
		col, \
		test_xy, \
		&tovi_ptr, \
		&trvr_ptr, \
		&grnd_ptr \
	)) goto unwind; \
} while (0)

#define visit_initial_col() do { \
	col = map_get_col_xy(map, xy); \
	if (!(col & (1LLU << z))) \
		return; \
\
	col = isolate_col(col, z); \
\
	cull->traversed[xy] = col; \
	cull->traversed_xy[trvr_ptr++] = xy; \
	if (cull->grounded[xy] == 0) \
		cull->grounded_xy[grnd_ptr++]  = xy; \
\
	/* TODO: can water fall on buggyspades? */ \
	if (col & (3LLU << (MAP_Z - 2))) \
		goto unwind; \
} while (0)

void map_cull_xy(Map map, CullState cull, long xy, int z)
{
	int grnd_ptr = cull->grnd_ptr;
	int trvr_ptr = 0;
	int tovi_ptr = 0;

	MapCol col;
	visit_initial_col();

	while (1) {
		if (xy % MAP_X > 0)
			visit(xy - 1);
		if (xy % MAP_X < MAP_X - 1)
			visit(xy + 1);
		if (xy >= MAP_X)
			visit(xy - MAP_X);
		if (xy < MAP_XY - MAP_X)
			visit(xy + MAP_X);

		if (tovi_ptr == 0)
			break;

		xy  = cull->to_visit_xy[--tovi_ptr];
		col = cull->traversed[xy];

		cull->visiting[xy/8] &= ~(1 << (xy & 7));
	}

	cull->grnd_ptr = grnd_ptr;
	return unwind_floating(map, cull, trvr_ptr);

	unwind:
	cull->grnd_ptr = grnd_ptr;
	return unwind_grounded(cull, trvr_ptr);
}

void map_cull(Map map, CullState cull, int x, int y, int z)
{
	map_cull_xy(map, cull, map_to_xy(x, y), z);
}
