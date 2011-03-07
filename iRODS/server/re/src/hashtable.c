/* For copyright information please refer to files in the COPYRIGHT directory
 */
#include "hashtable.h"
/**
 * returns NULL if out of memory
 */
struct bucket *newBucket(char* key, void* value) {
	struct bucket *b = (struct bucket *)malloc(sizeof(struct bucket));
	if(b==NULL) {
		return NULL;
	}
	b->next = NULL;
	b->key = key;
	b->value = value;
	return b;
}


/**
 * returns NULL if out of memory
 */
Hashtable *newHashTable(int size) {
	Hashtable *h = (Hashtable *)malloc(sizeof (Hashtable));
	if(h==NULL) {
		return NULL;
	}
	h->size = size;
	h->buckets = (struct bucket **)malloc(sizeof(struct bucket *)*size);
	if(h->buckets == NULL) {
		free(h);
		return NULL;
	}
	int i;
	for(i=0;i<size;i++) {
		h->buckets[i]=NULL;
	}
	h->len = 0;
	return h;
}
/**
 * the key is duplicated but the value is not duplicated.
 * MM: the key is managed by hashtable while the value is managed by the caller.
 * returns 0 if out of memory
 */
int insertIntoHashTable(Hashtable *h, char* key, void *value) {
//    printf("insert %s=%s\n", key, value==NULL?"null":"<value>");
	struct bucket *b = newBucket(strdup(key), value);
	if(b==NULL) {
		return 0;
	}
	unsigned long hs = hash(key);
	unsigned long index = hs%h->size;
	if(h->buckets[index] == NULL) {
		h->buckets[index] = b;
	} else {
		struct bucket *b0 = h->buckets[index];
		while(b0->next!=NULL)
			b0=b0->next;
		b0->next = b;
	}
	h->len ++;
	return 1;
}
/**
 * update hash table returns the pointer to the old value
 */
void* updateInHashTable(Hashtable *h, char* key, void* value) {
	unsigned long hs = hash(key);
	unsigned long index = hs%h->size;
	if(h->buckets[index] != NULL) {
		struct bucket *b0 = h->buckets[index];
		while(b0!=NULL) {
			if(strcmp(b0->key, key) == 0) {
				void* tmp = b0->value;
				b0->value = value;
				return tmp;
				// free not the value
			}
                        b0=b0->next;
		}
	}
	return NULL;
}
/**
 * delete from hash table
 */
void *deleteFromHashTable(Hashtable *h, char* key) {
	unsigned long hs = hash(key);
	unsigned long index = hs%h->size;
	void *temp = NULL;
	if(h->buckets[index] != NULL) {
            struct bucket *b0 = h->buckets[index];
            if(strcmp(b0->key, key) == 0) {
                    h->buckets[index] = b0->next;
                    temp = b0->value;
                    free(b0);
                    h->len --;
            } else {
                while(b0->next!=NULL) {
                    if(strcmp(b0->next->key, key) == 0) {
                        struct bucket *tempBucket = b0->next;
                        temp = b0->next->value;
                        b0->next = b0->next->next;
                        free(tempBucket);
                        h->len --;
                        break;
                    }
                }
            }
	}

	return temp;
}
/**
 * returns NULL if not found
 */
void* lookupFromHashTable(Hashtable *h, char* key) {
	unsigned long hs = hash(key);
	unsigned long index = hs%h->size;
	struct bucket *b0 = h->buckets[index];
	while(b0!=NULL) {
		if(strcmp(b0->key,key)==0) {
			return b0->value;
		}
		b0=b0->next;
	}
	return NULL;
}
/**
 * returns NULL if not found
 */
struct bucket* lookupBucketFromHashTable(Hashtable *h, char* key) {
	unsigned long hs = hash(key);
	unsigned long index = hs%h->size;
	struct bucket *b0 = h->buckets[index];
	while(b0!=NULL) {
		if(strcmp(b0->key,key)==0) {
			return b0;
		}
		b0=b0->next;
	}
	return NULL;
}
struct bucket* nextBucket(struct bucket *b0, char* key) {
    b0 = b0->next;
    while(b0!=NULL) {
        if(strcmp(b0->key,key)==0) {
            return b0;
        }
        b0=b0->next;
    }
    return NULL;
}

void deleteHashTable(Hashtable *h, void (*f)(void *)) {
	int i;
	for(i =0;i<h->size;i++) {
		struct bucket *b0 = h->buckets[i];
		if(b0!=NULL)
			deleteBucket(b0,f);
	}
        free(h->buckets);
        free(h);
}
void deleteBucket(struct bucket *b0, void (*f)(void *)) {
		if(b0->next!=NULL) {
			deleteBucket(b0->next, f);
		}
                // todo do not delete keys if they are allocated in regions
                free(b0->key);
		f(b0->value);
		free(b0);
}
void nop(void *a) {
}




unsigned long B_hash(unsigned char* string) { // Bernstein hash
	unsigned long hash = HASH_BASE;
	while(*string!='\0') {
	    hash = ((hash << 5) + hash) + (int)*string;
        string++;
	}
	return hash;

}
unsigned long sdbm_hash(char* str) { // sdbm
        unsigned long hash = 0;
        while (*str!='\0') {
            hash = ((int)*str) + (hash << 6) + (hash << 16) - hash;
			str++;
		}

        return hash;
}

#include "utils.h"
#include "index.h"

void dumpHashtableKeys(Hashtable *t) {
    int i;
    for(i = 0;i<t->size;i++) {
        struct bucket *b=t->buckets[i];
        while(b!=NULL) {
            writeToTmp("htdump", b->key);
            writeToTmp("htdump", "\n");
            b= b->next;
        }
    }
}

