#include "kernel/types.h"
#include "user/user.h"
#include "memory_management.h"

#define NULL (void *)0
#define INVALID (void *)-1
#define BLOCKSIZE 4096
#define BLOCKROUNDUP(sz)  (((sz)+BLOCKSIZE-1) & ~(BLOCKSIZE-1))
#define ALIGN(sz)  (((sz)+sizeof(long)-1) & ~(sizeof(long)-1))
#define ABS(n)  (n < 0 ? n * -1 : n)

header *base = NULL;

void *
_malloc(int size) {
    if (size <= 0)
        return NULL;

    // Align
    size = ALIGN(size);

    int spaceLeft = 0;
    header *space = base;
    header *bestFit = NULL;
    int bestDiff = -1;
    void *new;

    if (base != NULL) {
        // Find best header to allocate ahead of
        while (space->s.nextHeader != NULL) {
            if (space->s.size < 0) {
                int diff = -space->s.size - size;
                if (diff >= 0) {
                    if (bestFit == NULL) {
                        bestFit = space;
                        bestDiff = diff;
                    } else {
                        if (diff < bestDiff) {
                            bestFit = space;
                            bestDiff = diff;
                        }
                    }
                }
            }
            space = space->s.nextHeader;
        }

        // Find space until end of page
        long top = (long)(space + 1) + space->s.size;
        spaceLeft = BLOCKROUNDUP(top) - top;
    }

    if (bestFit == NULL) {
        // Create new block(s)
        if (spaceLeft < size + sizeof(header)) {
            int sizeRequired = size + sizeof(header) - spaceLeft;
            int numPages = (sizeRequired / BLOCKSIZE) + (sizeRequired % BLOCKSIZE != 0);
            if ((new = sbrk(BLOCKSIZE * numPages)) == INVALID)
                return NULL;

            // Start linked list
            if (space == NULL)
                space = (header*)new;
        }
    }

    // Determine new header offset
    long offset = 0;
    if (bestFit != NULL) {
        space = bestFit;
        offset = sizeof(header) + size;
    } else if (base != NULL) {
        offset = sizeof(header) + space->s.size;
    }
    
    header *newHeader = (header *)((long)space + offset);

    // Update linked list
    if (bestFit == NULL) {
        newHeader->s.nextHeader = NULL;
        newHeader->s.size = size;
    }
    else {
        newHeader->s.nextHeader = space->s.nextHeader;
        newHeader->s.size = space->s.size + size;
        space->s.size = size;
    }
    if (base == NULL)
        base = newHeader;
    else
        space->s.nextHeader = newHeader;

    // Return start of allocation
    if (bestFit == NULL)
        return newHeader + 1; 
    else 
        return space + 1;
}

void 
_free(void *ptr) {
    if (ptr == NULL)
        return;        

    // Free the allocation
    ((header *)(ptr)- 1)->s.size *= -1;

    header *h = base, *end = base;
    header *lastNotFree = NULL;

    while (h != NULL) {
        // Merge if gap
        header *next = h->s.nextHeader;
        while (h->s.size < 0 && next != NULL && next->s.size < 0) {
            h->s.size += next->s.size;
            h->s.nextHeader = next = next->s.nextHeader;  
        }

        // Record last header not free
        if (h->s.size > 0) 
            lastNotFree = h;
    
        end = h;
        h = h->s.nextHeader;
    }


    // Base free
    long top = (long)base;
    if (lastNotFree == NULL)
        base = NULL;
    // Clip top
    else {
        top = (long)lastNotFree + lastNotFree->s.size + sizeof(header);
        if (lastNotFree->s.nextHeader != NULL) {
            lastNotFree->s.nextHeader = NULL;
        }
    }

    // Give top back to OS
    long heapTop = BLOCKROUNDUP((long)(end + 1) + ABS(end->s.size));
    long freeSize = heapTop - BLOCKROUNDUP(top);
    sbrk(-freeSize);
}