#ifndef PoVCuqlRmLMoGZfyVxstQEhNCNHwDLvT
#define PoVCuqlRmLMoGZfyVxstQEhNCNHwDLvT

#include "map.h"
#include <stddef.h>

/* Non-streaming-exclusive VXL decode state */
struct VXLState {
	int xy;

	int top_start;
	int top_end;
	int bottom_len;
	int last_span;

	Map map;

	/* This'll include the next span hdr in the length
	 * if not the last span.
	 */
	int span_nbytes;
};

struct VXLStream {
	struct VXLState state;

	int part;

	/* This'll include the next span hdr at the end
	 * of it if not the last span, hence the + 1.
	 */
	int span_buf_nbytes;
	unsigned char span_buf[(MAP_Z + 1) * 4];
};

int vxl_read(Map map, const void *src, size_t nbytes);

void vxl_stream_read_open(struct VXLStream *stream, Map map);
int vxl_stream_read(struct VXLStream *stream, const void *src, size_t nbytes);

#endif
