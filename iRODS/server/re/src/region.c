/* For copyright information please refer to files in the COPYRIGHT directory
 */
#include "region.h"
#include <string.h>

/* utility function */
struct region_node *make_region_node(size_t is) {
	struct region_node *node = (struct region_node *)malloc(sizeof(Region));
	if(node == NULL) {
		return NULL;
	}

	node->block = (unsigned char *)malloc(is);
        memset(node->block, 0, is);
	if(node->block == NULL) {
		free(node);
		return NULL;
	}

	node->size = is;
	node->used = 0;
	node->next = NULL;

	return node;
}
Region *make_region(size_t is, jmp_buf *label) {
	Region *r = (Region *)malloc(sizeof(Region));
	if(r == NULL)
		return NULL;

	if(is == 0)
		is = DEFAULT_BLOCK_SIZE;
	struct region_node *node = make_region_node(is);
	if(node == NULL) {
		free(r);
		return NULL;
	}

	r->head = r->active = node;
        r->label = label;
        r->error.code = 0; /* set no error */
	return r;
}
unsigned char *region_alloc_nodesc(Region *r, size_t s) {
	if(s > r->active->size - r->active->used) {
            int blocksize;
            if(s > DEFAULT_BLOCK_SIZE) {
                blocksize=s;

            } else {
                blocksize= DEFAULT_BLOCK_SIZE;
            }
            struct region_node *next = make_region_node(blocksize);
            if(next == NULL) {
                if(r->label == NULL) { /* no error handler */
                    return NULL;
                } else { /* with error handler */
                    longjmp(*(r->label), -1);
                }
            }
            r->active->next = next;
            r->active = next;

	}

	size_t alloc_size =
                s>DEFAULT_BLOCK_SIZE?
                    s:
                    roundToAlignment(s);
	unsigned char *pointer = r->active->block + r->active->used;
	r->active->used+=alloc_size;
	return pointer;

}
void *region_alloc(Region *r, size_t size) {
    unsigned char *mem = region_alloc_nodesc(r, size + sizeof(RegionDesc));
    ((RegionDesc *)mem)->region = r;
    ((RegionDesc *)mem)->size = size;
    ((RegionDesc *)mem)->del = 0;
    return mem + sizeof(RegionDesc);
}
void region_free(Region *r) {
	while(r->head!=NULL) {
		struct region_node *node = r->head;
		r->head = node->next;
		free(node->block);
		free(node);
	}
        free(r->label);
	free(r);
}
size_t region_size(Region *r) {
	size_t s = 0;
	struct region_node *node = r->head;
	while(node!=NULL) {
		s += node->used;
		node = node->next;
	}
	return s;
}

/* tests */
void assert(int res) {
	if(!res) {
		printf("error");
	}
}
void test() {
	Region *r = make_region(0, NULL);
	assert(region_alloc(r, 16)!=NULL);
	assert(region_alloc(r, 20)!=NULL);
	assert(region_alloc(r,1000)!=NULL);
	assert(region_alloc(r,10000)!=NULL);
	assert(region_alloc(r,16)!=NULL);
	assert(region_alloc(r,10)!=NULL);
	assert(region_alloc(r,1024-16-10)!=NULL);
	region_free(r);
}
