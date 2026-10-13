#ifndef PVX2_WRITE_H
#define PVX2_WRITE_H

#include "map.h"
#include <stddef.h>

size_t vxl_write(void *dst, struct MapBuf *, int xy, int cols);

#endif
