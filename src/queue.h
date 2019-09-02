/** @file
 * Structures and functions providing queue operations.
 */

#ifndef MAP_QUEUE_H
#define MAP_QUEUE_H

#include <stdbool.h>
#include "global_declarations.h"

/// check if a queue is empty
bool queueEmpty(const Heap *heap);
/// add a record to the queue
bool queuePush(Heap *heap, Road *road, City *city, size_t distance, int minYear);
/// initialize a queue
Heap *queueInit(void);
/// take a record from the queue
Road *queuePop(Heap *heap, size_t *distance, int *minYear, City **pCity);
/// destroy a queue
void queueDestroy(Heap **pHeap);

#endif //MAP_QUEUE_H
