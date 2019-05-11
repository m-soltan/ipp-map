#ifndef MAP_QUEUE_H
#define MAP_QUEUE_H

#include <stdbool.h>
#include "global_declarations.h"

bool queueEmpty(const Heap *heap);
bool queuePush(Heap *heap, Road *road, City *city, size_t distance, int minYear);
Heap *queueInit(void);
Road *queuePop(Heap *heap, size_t *distance, int *minYear, City **pCity);
void queueDestroy(Heap **pHeap);

#endif //MAP_QUEUE_H
