#ifndef IzlnvsfFxjMISdrcNNnmUgIctrFClNxl
#define IzlnvsfFxjMISdrcNNnmUgIctrFClNxl

#define MAP_X 512
#define MAP_Y 512
#define MAP_Z  64

#define MAP_SOLIDBUF_SIZ ((MAP_X * MAP_Y * MAP_Z + 7) / 8)
#define MAP_COLORBUF_SIZ  (MAP_X * MAP_Y * MAP_Z * 3)

struct MapBuf {
	unsigned char *solidbuf;
	unsigned char *colorbuf;
};

#endif
