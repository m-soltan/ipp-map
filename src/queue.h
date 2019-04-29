#ifndef MAP_QUEUE_H
#define MAP_QUEUE_H

#include <stdbool.h>
#include "global_declarations.h"

bool queueEmpty(const Heap *heap);
bool queuePush(Heap *heap, City *city, City *prev, size_t distance, int minYear);
City *queuePop(Heap *heap, size_t *distance, int *minYear, City **prev);
Heap *queueInit(void);
void queueDestroy(Heap **pHeap);

#endif //MAP_QUEUE_H
