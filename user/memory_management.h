#ifndef MEMORY_MANAGEMENT
#define MEMORY_MANAGEMENT

typedef long Align;
typedef union header {
    struct {
        union header* nextHeader;
        int size;
    } s;
    Align x;
} header;

void *_malloc(int size);
void _free(void *ptr);

#endif // MEMORY_MANAGEMENT