#ifndef IzlnvsfFxjMISdrcNNnmUgIctrFClNxl
#define IzlnvsfFxjMISdrcNNnmUgIctrFClNxl

#define VXL_X 512
#define VXL_Y 512
#define VXL_Z  64

#define VXL_SOLIDBUF_SIZ ((VXL_X * VXL_Y * VXL_Z + 7) / 8)
#define VXL_COLORBUF_SIZ  (VXL_X * VXL_Y * VXL_Z * 3)

struct VXLMapBuf {
	unsigned char *solidbuf;
	unsigned char *colorbuf;
};

#endif
