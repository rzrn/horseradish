cdef extern from "pvx2/map.h":
    enum:
        MAP_X
        MAP_Y
        MAP_Z
        MAP_XY
        MAP_XYZ

    struct MapBuf:
        unsigned char * colorbuf

    long map_to_xy(int x, int y) nogil

    unsigned char * map_get_color_xy(MapBuf *, long      xy, int z) nogil
    unsigned char * map_get_color   (MapBuf *, int x, int y, int z) nogil

    void map_set_color_xy(MapBuf *, long      xy, int z, const void *, long len) nogil
    void map_set_color   (MapBuf *, int x, int y, int z, const void *, long len) nogil

    void map_clear(MapBuf *) nogil
    void map_copy (MapBuf *, const MapBuf *) nogil

    void map_create_range_xy(MapBuf *, long      xy, int z, long len) nogil
    void map_create_range   (MapBuf *, int x, int y, int z, long len) nogil
    void map_create_xy      (MapBuf *, long      xy, int z) nogil
    void map_create         (MapBuf *, int x, int y, int z) nogil

    void map_destroy_range_xy(MapBuf *, long      xy, int z, long len) nogil
    void map_destroy_range   (MapBuf *, int x, int y, int z, long len) nogil
    void map_destroy_xy      (MapBuf *, long      xy, int z) nogil
    void map_destroy         (MapBuf *, int x, int y, int z) nogil

    int map_is_solid_xy(MapBuf *, long      xy, int z) nogil
    int map_is_solid   (MapBuf *, int x, int y, int z) nogil

    int map_get_floor_xy(MapBuf *, long      xy, int z) nogil
    int map_get_floor   (MapBuf *, int x, int y, int z) nogil
    int map_get_top_xy  (MapBuf *, long      xy) nogil
    int map_get_top     (MapBuf *, int x, int y) nogil

cdef extern from "pvx2/read.h":
    int vxl_read(MapBuf *, const void * src, size_t nbytes) nogil

cdef extern from "pvx2/write.h":
    size_t vxl_write(void *, MapBuf *, int xy, int cols) nogil

cdef extern from "pvx2/cull.h":
    struct CullBuf:
        pass

    int map_cull(MapBuf *, CullBuf *, int x, int y, int z) nogil

    void map_cull_end(CullBuf *) nogil

cdef inline bool is_valid_position(int x, int y, int z):
    return 0 <= x and x < 512 and 0 <= y and y < 512 and 0 <= z and z < 64

cdef class VXLData:
    cdef MapBuf * mapbuf
    cdef CullBuf * cullbuf
    cdef unsigned char * colorbuf

    cdef load_bytes(self, bytes)

    cpdef get_solid(self, int x, int y, int z)
    cpdef get_color(self, int x, int y, int z)
    cpdef tuple get_random_point(self, int x1, int y1, int x2, int y2)
    cpdef int get_z(self, int x, int y, int start = ?)
    cpdef int get_height(self, int x, int y)
    cpdef tuple get_safe_coords(self, int x, int y, int z)
    cpdef bint has_neighbors(self, int x, int y, int z)
    cpdef bint is_surface(self, int x, int y, int z)
    cpdef list get_neighbors(self, int x, int y, int z)
    cpdef int cull(self, int x, int y, int z)
    cpdef bint build_point(self, int x, int y, int z, tuple color)
    cpdef bint set_column_fast(self, int x, int y, int start_z,
        int end_z, int end_color_z, int color)
