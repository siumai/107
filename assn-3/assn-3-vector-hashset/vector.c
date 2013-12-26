#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <search.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    v->elemSize = elemSize;
    v->logLength = 0;
    v->allocLength = initialAllocation;
    v->growSize = initialAllocation;
    v->elems = malloc(initialAllocation * elemSize);
    v->freeFn = freeFn;
    assert(v->elems != NULL);
}

void VectorDispose(vector *v)
{
    if (v->freeFn != NULL) {
        for (int i = 0; i < v->logLength; i++) {
            v->freeFn((char *)v->elems + i*v->elemSize);
        }
    }
    
    free(v->elems);
}

int VectorLength(const vector *v)
{
    return v->logLength;
}

void *VectorNth(const vector *v, int position)
{
    if (position >= v->logLength) {
        fprintf(stdout, "\n loglength %d: vector nth failed %d: ",v->logLength, position);
    }
    assert(position>=0 && position < v->logLength);
    
    return (char *)v->elems + position*v->elemSize;
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    void *elmPtr = VectorNth(v, position);
    if (v->freeFn != NULL) {
        v->freeFn(elmPtr);
    }
    
    memcpy(elmPtr, elemAddr, v->elemSize);
}

static void VectorGrow(vector *v)
{
    v->allocLength += v->growSize;
    v->elems = realloc(v->elems, v->allocLength*v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    if (v->logLength == v->allocLength) {
        VectorGrow(v);
    }
    
    void *elmPtr = (char *)v->elems + position*v->elemSize;
    memmove((char *)elmPtr+v->elemSize, elmPtr, (v->logLength - position)*v->elemSize);
    memcpy(elmPtr, elemAddr, v->elemSize);
    v->logLength++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
    if (v->logLength == v->allocLength) {
        VectorGrow(v);
    }
    
    void *elmPtr = (char *)v->elems + v->logLength*v->elemSize;
    memcpy(elmPtr, elemAddr, v->elemSize);
    v->logLength++;
}

void VectorDelete(vector *v, int position)
{
    if (position < v->logLength - 1) {
        void *elmPtr = VectorNth(v, position);
        memmove(elmPtr, (char *)elmPtr+v->elemSize, (v->logLength - 1 - position)*v->elemSize);
    }
    v->logLength--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    assert(compare!=NULL);
    qsort(v->elems, v->logLength, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    assert(mapFn!=NULL);
    
    for (int i=0; i<v->logLength; i++) {
        void *elmPtr = VectorNth(v, i);
        mapFn(elmPtr, auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
    if (v->logLength == 0) {
        return kNotFound;
    }
    
    void *startPtr = VectorNth(v, startIndex);
    size_t size = v->logLength - startIndex;
    void *found;
    if (isSorted == true) {
        found = bsearch(key, startPtr, size, v->elemSize, searchFn);
    } else {
        found = lfind(key, startPtr, &size, v->elemSize, searchFn);
    }
    
    if (found!=NULL) {
        return (((char *)found) - ((char *)(v->elems)))/v->elemSize;
    }
    return kNotFound;

}
