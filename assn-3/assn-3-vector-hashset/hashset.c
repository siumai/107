#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
    h->elemSize = elemSize;
    h->numBuckets = numBuckets;
    h->hashfn = hashfn;
    h->comparefn = comparefn;
    h->freefn = freefn;
    size_t vsize = sizeof(vector);
    h->hashsets = malloc(numBuckets * vsize);
    for (int i=0; i<numBuckets; i++) {
        vector *v = (vector *)((char *)h->hashsets + vsize*i);
        VectorNew(v, elemSize, freefn, 10);
    }
}

void HashSetDispose(hashset *h)
{
    if (h->freefn) {
        size_t vsize = sizeof(vector);
        for (int i=0; i<h->numBuckets; i++) {
            vector *v = (vector *)((char *)h->hashsets + vsize*i);
            VectorDispose(v);
        }
    }
    
    free(h->hashsets);
}

int HashSetCount(const hashset *h)
{
    int count = 0;
    size_t vsize = sizeof(vector);
    for (int i=0; i<h->numBuckets; i++) {
        vector *v = (vector *)((char *)h->hashsets + vsize*i);
        count += VectorLength(v);
    }
    return count;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
    size_t vsize = sizeof(vector);
    for (int i=0; i<h->numBuckets; i++) {
        vector *v = (vector *)((char *)h->hashsets + vsize*i);
        VectorMap(v, mapfn, auxData);
    }
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
    assert(elemAddr != NULL);
    size_t vsize = sizeof(vector);
    int bucket = h->hashfn(elemAddr, h->numBuckets);
    assert(bucket>=0 && bucket<h->numBuckets);
    vector *v = (vector *)((char *)h->hashsets + vsize*bucket);
    int index = VectorSearch(v, elemAddr, h->comparefn, 0, false);
    if (index!=-1) {
        VectorReplace(v, elemAddr, index);
    } else {
        VectorAppend(v, elemAddr);
    }
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
    assert(elemAddr!=NULL);
    size_t vsize = sizeof(vector);
    int bucket = h->hashfn(elemAddr, h->numBuckets);
    assert(bucket>=0 && bucket<h->numBuckets);
    vector *v = (vector *)((char *)h->hashsets + vsize*bucket);
    int index = VectorSearch(v, elemAddr, h->comparefn, 0, false);
    if (index!=-1) {
        return VectorNth(v, index);
    } else {
        return NULL;
    }
}
