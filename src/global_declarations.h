/** @file
 * Declarations used in most files throughout the projects
 */

#ifndef MAP_GLOBAL_DECLARATIONS_H
#define MAP_GLOBAL_DECLARATIONS_H

/// all route ids must be lower than this
#define ROUTE_LIMIT 1000

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

//! @cond
typedef struct City City;
typedef struct CityInfo CityInfo;
typedef struct CityMap CityMap;
typedef struct Heap Heap;
typedef struct NameList NameList;
typedef struct Map Map;
typedef struct Road Road;
typedef struct RoadMap RoadMap;
typedef struct RoadInfo RoadInfo;
typedef struct Trie Trie;
typedef struct Trunk Trunk;
//! @endcond

/// Lists names of the cities that the Route will go through.
struct NameList {
	/// the names stored
	const char **v;
	/// number of items in the list
	size_t length;
};

//! @cond
struct RoadInfo {
	const char *city1, *city2;
	int builtYear;
	unsigned length;
	RoadMap *roadMap;
};
//! @endcond

#endif //MAP_GLOBAL_DECLARATIONS_H
