#ifndef MAP_QUEUE_H
#define MAP_QUEUE_H

#include <stdbool.h>
#include "city.h"

typedef struct Heap Heap;

//debug
void qTest(void);

bool queueEmpty(const Heap *heap);
bool queuePush(Heap *heap, City *city, size_t distance);
City *queuePeek(Heap *heap);
City *queuePop(Heap *heap, size_t *distance);
Heap *queueInit(void);

#endif //MAP_QUEUE_H
