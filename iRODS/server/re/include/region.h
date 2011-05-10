/* For copyright information please refer to files in the COPYRIGHT directory
 */
#ifndef _REGION_H
#define _REGION_H

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#define DEFAULT_BLOCK_SIZE 1024
/* the alignment in the region in bytes */
#define ALIGNMENT 4

struct error {
    int code;
    char msg[1024];
    void *obj; /* a generic point to an object which is the source of the error */
};

struct region_node {
	unsigned char *block; /* pointer to memory block */
	size_t size; /* size of the memory block in bytes */
	size_t used; /* used bytes of the memory block */
	struct region_node *next; /* pointer to the next region */

};


typedef struct region {
	struct region_node *head, *active;
        jmp_buf *label;
        struct error error;
} Region;

typedef struct region_desc {
    Region *region;
    size_t size;
    int del;
} RegionDesc;

#define IN_REGION(x,r) (((RegionDesc *)(((unsigned char*)(x))-sizeof(RegionDesc)))->region == (r))
#define SET_DELETE(x) (((RegionDesc *)(((unsigned char*)(x))-sizeof(RegionDesc)))->del=1)
#define DELETED(x) (((RegionDesc *)(((unsigned char*)(x))-sizeof(RegionDesc)))->del)
#define SIZE(x) (((RegionDesc *)(((unsigned char*)(x))-sizeof(RegionDesc)))->size)

/* create a region with initial size is */
/* if s == 0 then the initial size is DEFAULT_BLOCK_SIZE */
/* returns NULL if it runs out of memory */
Region *make_region(size_t is, jmp_buf *label);
/* allocation s bytes in region r */
/* return NULL if it runs out of memory */
void *region_alloc(Region *r, size_t s);
/* free region r */
void region_free(Region *r);
#endif
