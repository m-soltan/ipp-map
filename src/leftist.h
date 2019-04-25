#ifndef MAP_LEFTIST_H
#define MAP_LEFTIST_H

#include <stdbool.h>
#include "city.h"

typedef struct Leftist Leftist;

bool isEmpty(const Leftist *t);
City *leftistPeek(Leftist *t);
City *leftistPop(Leftist *t);
bool leftistPush(Leftist *t, City *city, size_t distance);

#endif //MAP_LEFTIST_H
