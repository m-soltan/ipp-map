#ifndef MAP_GLOBAL_DECLARATIONS_H
#define MAP_GLOBAL_DECLARATIONS_H

#define ROUTE_LIMIT 1000

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct City City;
typedef struct CityInfo CityInfo;
typedef struct CityMap CityMap;
typedef struct Heap Heap;
typedef struct NameList NameList;
/** @brief A structure storing the trunk road map.
 */
typedef struct Map Map;
typedef struct Road Road;
typedef struct RoadMap RoadMap;
typedef struct RoadInfo RoadInfo;
typedef struct Trie Trie;
typedef struct Trunk Trunk;

struct NameList {
	const char **v;
	size_t length;
};

struct RoadInfo {
	const char *city1, *city2;
	int builtYear;
	unsigned length;
	RoadMap *roadMap;
};

#endif //MAP_GLOBAL_DECLARATIONS_H
