#ifndef MAP_TRUNK_H
#define MAP_TRUNK_H
#include "city.h"
//#include "road.h"

typedef struct List List;
typedef struct Trunk Trunk;

Trunk *listPop(List **l);
bool listPush(List **l, Trunk *value);

#endif //MAP_TRUNK_H
