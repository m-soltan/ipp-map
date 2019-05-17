#ifndef MAP_GLOBAL_DECLARATIONS_H
#define MAP_GLOBAL_DECLARATIONS_H

#define ROUTE_ID_MAX 999
#define ROUTE_LIMIT 1000

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// TODO: remove
// a counter to force a full rebuild
// 13

typedef struct City City;
typedef struct CityMap CityMap;
typedef struct Heap Heap;
/**
 * A structure storing the trunk road map.
 */
typedef struct Map Map;
typedef struct Road Road;
typedef struct RoadInfo RoadInfo;
typedef struct Trie Trie;
typedef struct Trunk Trunk;

#endif //MAP_GLOBAL_DECLARATIONS_H
