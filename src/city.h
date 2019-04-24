#ifndef MAP_CITY_H
#define MAP_CITY_H
#include <stdbool.h>
#include "trie.h"

typedef struct City City;
typedef struct Road Road;
typedef struct Trie Trie;

bool cityMakeSpace(City *city);
bool cityAddRoad(City *city, Road *road);
bool roadDestroy(Road *road);

bool roadExtend(Trie *t, City *city, const char *str);
bool roadFind(const City *city1, const City *city2);
bool roadInsert(Trie *t, Road **road);
bool roadLink(City *city1, City *city2);
City *cityInit(Road *road, const char *name);
Road *roadInit(const char *city1, const char *city2, unsigned len, int year);
void cityDetach(City *city, const Road *road);

#endif //MAP_CITY_H
