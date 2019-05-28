#define SRC_SEEK(src,pos) ((*(src)->seek_func)((pos), (src)->arg))
#define SRC_READ(src,buf,cnt) ((*(src)->read_func)((buf),(cnt),(src)->arg))
