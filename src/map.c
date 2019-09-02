#include "city.h"
#include "city_map.h"
#include "map.h"
#include "queue.h"
#include "road.h"
#include "trunk.h"
#include "trie.h"

/// A structure storing the trunk road map.
struct Map {
	/// a structure storing the cities in the map
	CityMap *cities;
	/// a structure storing the roads in the map
	RoadMap *roads;
	/// all routes in the map, indexed by id
	Trunk *routes[ROUTE_LIMIT];
	/// a structure storing references to cities for fast lookup by name
	Trie *trie;
};

//! @cond
static bool addFromList(Map *map, NameList list, const int *years, const unsigned *roadLengths);
static bool correctRoute(unsigned routeId, const char *name1, const char *name2);
static bool destroyRoad(Map *map, Road *road);
static bool invalidId(unsigned routeId);
static bool namesAreCorrect(const char *city1, const char *city2);
static bool nameError(const char *str);
static bool testExistingRoads(Map *map, const char **names, const unsigned *roadLengths, size_t length);
static bool testNameUniqueness(const char **names, size_t length);
static bool testRoute(Map *map, const char **names, const int *years, const unsigned *roadLengths, size_t length);
static bool testYears(Map *map, const char **names, const int *years, size_t length);
static void destroyTrunks(Map *map);
static void repairFromList(Map *map, NameList list, const int *years);
static Road *find(Trie *trie, const char *city1, const char *city2);

#ifndef NDEBUG
static bool testInvariants(Map *map);
static bool *whichTrunks(const Map *map);
#endif // NDEBUG
//! @endcond

Map *newMap(void) {
	Map *ans = calloc(1, sizeof(Map));
	if (ans) {
		ans->trie = trieInit();
		if (ans->trie) {
			ans->cities = cityMapInit();
			if (ans->cities) {
				ans->roads = roadMapInit();
				if (ans->roads) {
					assert(testInvariants(ans));
					return ans;
				}
				free(ans->cities);
			}
			free(ans->trie);
		}
		free(ans);
	}
	return NULL;
}

void deleteMap(Map *map) {
	if (map == NULL)
		return;
	assert(map->routes[0] == NULL);
	destroyTrunks(map);
	cityMapDestroy(&map->cities);
	roadMapDestroy(&map->roads);
	trieDestroy(&map->trie);
	free(map);
}

bool addRoad(Map *map, const char *city1, const char *city2, unsigned length, int builtYear) {
	bool ans = false;
	if (builtYear == 0 || length == 0 || !namesAreCorrect(city1, city2))
		return false;
	RoadInfo info = (RoadInfo) {
			.builtYear = builtYear,
			.length = length,
			.roadMap = map->roads,
	};
	City *c1 = trieFind(map->trie, city1), *c2 = trieFind(map->trie, city2);
	size_t count1 = SIZE_MAX - 1, count2 = SIZE_MAX - 1;
	if (c1 && c2) {
		count1 = cityGetRoadCount(c1);
		count2 = cityGetRoadCount(c2);
		ans = roadLink(map->roads, c1, c2, length, builtYear);
	} else {
		info.city1 = (c1 ? NULL : city1);
		info.city2 = (c2 ? NULL : city2);
		if (c1) {
			count1 = cityGetRoadCount(c1);
			ans = roadExtend(map->cities, map->trie, c1, info);
		}
		if (c2) {
			count2 = cityGetRoadCount(c2);
			ans = roadExtend(map->cities, map->trie, c2, info);
		}
		if (!c1 && !c2)
			ans = roadLoneRoad(map->cities, map->trie, info);
	}
	if (ans && c1) {
		(void) count1;
		assert(cityGetRoadCount(c1) == count1 + 1);
	}
	if (ans && c2) {
		(void) count2;
		assert(cityGetRoadCount(c2) == count2 + 1);
	}
	return ans;
}

bool repairRoad(Map *map, const char *city1, const char *city2, int repairYear) {
	bool ans;
	Road *r;
	r = find(map->trie, city1, city2);
	if (r == NULL)
		return false;
	ans = roadUpdate(r, repairYear);
	assert(testInvariants(map));
	return ans;
}

bool newRoute(Map *map, unsigned routeId, const char *city1, const char *city2) {
	City *c1, *c2;
	Trunk *route;
	c1 = trieFind(map->trie, city1);
	c2 = trieFind(map->trie, city2);
	if (!correctRoute(routeId, city1, city2))
		return false;
	if (c1 && c2 && map->routes[routeId] == NULL) {
		route = trunkBuild(c1, c2, map->cities, routeId);
		if (route) {
			if (trunkGetLength(route) < SIZE_MAX) {
				assert(routeId < ROUTE_LIMIT);
				map->routes[routeId] = route;
				trunkAttach(route);
				assert(trunkTest(route));
				assert(testInvariants(map));
				return true;
			}
			trunkFree(&route);
		}
	}
	return false;
}

bool extendRoute(Map *map, unsigned routeId, const char *city) {
	City *c;
	Trunk *extension, *route;
	if (invalidId(routeId) || nameError(city) || map->routes[routeId] == NULL)
		return false;
	c = trieFind(map->trie, city);
	route = map->routes[routeId];
	if (c == NULL || trunkHasCity(route, c))
		return false;
	extension = trunkExtend(map->cities, route, c);
	assert(testInvariants(map));
	if (extension == NULL)
		return false;
	map->routes[routeId] = extension;
	trunkAttach(extension);
	trunkFree(&route);
	assert(testInvariants(map));
	return true;
}

bool removeRoad(Map *map, const char *city1, const char *city2) {
	assert(testInvariants(map));
	bool ans;
	Road *r = find(map->trie, city1, city2);
	if (r == NULL)
		return false;
	ans = destroyRoad(map, r);
	assert(!map->routes[410] || trunkTest(map->routes[410]));
	if (ans)
		assert(find(map->trie, city1, city2) == NULL);
	assert(testInvariants(map));
	return ans;
}

char *routeDescriptionAux(Map *map, unsigned routeId) {
	char *ans = calloc(1, sizeof(char));
	if (ans) {
		if (!invalidId(routeId)) {
			Trunk *route = map->routes[routeId];
			if (route != NULL) {
				free(ans);
				ans = trunkDescription(route);
			}
		}
	}
	return ans;
}

Trunk **rebuildTrunks(CityMap *cityMap, Road *road, Trunk *trunks[ROUTE_LIMIT]) {
	const unsigned routeCount = roadRouteCount(road);
	unsigned ids[ROUTE_LIMIT];
	roadGetIds(road, ids);
	Trunk **replacements = calloc(routeCount, sizeof(Trunk *));
	if (replacements == NULL)
		return NULL;
	for (size_t i = 0; i < routeCount; ++i) {
		unsigned trunkId = ids[i];
		replacements[i] = trunkAddDetour(cityMap, trunks[trunkId], road);
		if (replacements[i] == NULL) {
			for (size_t j = 0; j < i; ++j) {
				trunkFree(&replacements[j]);
			}
			free(replacements);
			return NULL;
		}
	}
	return replacements;
}

bool routeFromList(
		Map *map,
		unsigned id,
		const char **names,
		const unsigned *rLengths,
		const int *years,
		size_t length
) {
	if (!testRoute(map, names, years, rLengths, length))
		return false;
	if (invalidId(id) || map->routes[id])
		return false;
	const size_t cityCount = cityMapGetLength(map->cities);
	const size_t roadCount = roadMapGetLength(map->roads);
	NameList list = (NameList) {.length = length, .v  = names};
	bool addSuccess = addFromList(map, list, years, rLengths);
	if (addSuccess) {
		Trunk *trunk = trunkMake(map->trie, id, list);
		if (trunk) {
			bool insertSuccess;
			if (cityCount == cityMapGetLength(map->cities)) {
				insertSuccess = true;
			} else {
				assert(cityCount < cityMapGetLength(map->cities));
				City *const *suffix = cityMapSuffix(map->cities, cityCount);
				insertSuccess = trieAddFromList(map->trie, list, suffix);
			}
			if (insertSuccess) {
				assert(!invalidId(id));
				map->routes[id] = trunk;
				repairFromList(map, list, years);
				trunkAttach(trunk);
				assert(trunkTest(trunk));
				return true;
			}
			trunkFree(&trunk);
		}
		cityMapTrim(map->cities, cityCount);
		roadMapTrim(map->roads, roadCount);
	}
	return false;
}

const char *getRouteDescription(Map *map, unsigned routeId) {
	return routeDescriptionAux(map, routeId);
}

bool removeRoute(Map *map, unsigned routeId) {
	if (invalidId(routeId))
		return false;
	if (map->routes[routeId] == NULL)
		return false;
	trunkDestroy(&map->routes[routeId]);
	return true;
}

//! @endcond
static bool destroyRoad(Map *map, Road *road) {
	City *city1, *city2;
	roadGetCities(road, &city1, &city2);
	if (roadRouteCount(road) > 0) {
		bool moveSuccess = roadMoveTrunks(map->cities, map->routes, road);
		assert(testInvariants(map));
		if (!moveSuccess)
			return false;
	}
	roadDetach(road, city1);
	roadDetach(road, city2);
	return true;
}

static bool namesAreCorrect(const char *city1, const char *city2) {
	bool ans = true;
	ans = ans && !nameError(city1);
	ans = ans && !nameError(city2);
	ans = ans && (strcmp(city1, city2) != 0);
	return ans;
}

static bool nameError(const char *str) {
	const size_t len = strlen(str);
	if (len == 0)
		return true;
	for (size_t i = 0; i < len; ++i) {
		char c = str[i];
		if (c > '\x00' && c < '\x20')
			return true;
		if (c == ';')
			return true;
	}
	return false;
}

static bool correctRoute(unsigned routeId, const char *name1, const char *name2) {
	bool ans = true;
	ans = ans && strcmp(name1, name2) != 0;
	ans = ans && !invalidId(routeId);
	return ans;
}

static bool invalidId(unsigned routeId) {
	return routeId < 1 || routeId >= ROUTE_LIMIT;
}

static bool testNameUniqueness(const char **names, size_t length) {
	bool ans = false;
	Trie *seen = trieInit();
	if (seen) {
		City *blank = cityDecoy();
		if (blank) {
			size_t i = 0;
			for (; i < length; ++i) {
				bool insertSuccess;
				if (trieFind(seen, names[i]))
					break;
				insertSuccess = trieInsert(seen, names[i], blank);
				if (!insertSuccess)
					break;
			}
			if (i == length)
				ans = true;
			cityDestroy(&blank);
		}
		trieDestroy(&seen);
	}
	return ans;
}

static bool testExistingRoads(Map *map, const char **names, const unsigned *roadLengths, size_t length) {
	for (size_t i = 1; i < length; ++i) {
		const char *city1 = names[i - 1], *city2 = names[i];
		Road *road = find(map->trie, city1, city2);
		if (road && (roadGetLength(road) != roadLengths[i - 1]))
			return false;
	}
	return true;
}

static bool testYears(Map *map, const char **names, const int *years, size_t length) {
	for (size_t i = 1; i < length; ++i) {
		City *city1, *city2;
		city1 = trieFind(map->trie, names[i - 1]);
		city2 = trieFind(map->trie, names[i]);
		if (city1 == NULL || city2 == NULL)
			continue;
		Road *road = cityFindRoad(city1, city2);
		if (road && roadGetYear(road) > years[i - 1])
			return false;
	}
	return true;
}

static bool testRoute(Map *map, const char **names, const int *years, const unsigned *roadLengths, size_t length) {
	bool ans = true;
	ans = ans && testExistingRoads(map, names, roadLengths, length);
	ans = ans && testNameUniqueness(names, length);
	ans = ans && testYears(map, names, years, length);
	return ans;
}

static bool addFromList(Map *map, NameList list, const int *years, const unsigned *roadLengths) {
	for (size_t i = 1; i < list.length; ++i) {
		const char *city1 = list.v[i - 1];
		const char *city2 = list.v[i];
		Road *road = find(map->trie, city1, city2);
		if (road == NULL) {
			bool addSuccess = addRoad(
					map,
					city1,
					city2,
					roadLengths[i - 1],
					years[i - 1]
			);
			if (!addSuccess) {
				assert(false);
				return false;
			}
			road = find(map->trie, city1, city2);
		}
		assert(road);
		bool reserveSuccess = roadReserve(road);
		if (!reserveSuccess) {
			assert(false);
			return false;
		}
	}
	return true;
}

static Road *find(Trie *trie, const char *city1, const char *city2) {
	City *c1, *c2;
	c1 = trieFind(trie, city1);
	c2 = trieFind(trie, city2);
	if (c1 && c2)
		return cityFindRoad(c1, c2);
	else
		return NULL;
}

static void destroyTrunks(Map *map) {
	for (size_t i = 1; i < ROUTE_LIMIT; ++i) {
		if (map->routes[i] != NULL)
			trunkDestroy(&map->routes[i]);
	}
}

static void repairFromList(Map *map, NameList list, const int *years) {
	for (size_t i = 1; i < list.length; ++i) {
		const char *city1 = list.v[i - 1];
		const char *city2 = list.v[i];
		bool success;
		assert(trieFind(map->trie, city1));
		assert(trieFind(map->trie, city2));
		Road *road = find(map->trie, city1, city2);
		assert(road);
		success = roadUpdate(road, years[i - 1]);
		(void) success;
		assert(success);
	}
}

// debug function used only in assertions
#ifndef NDEBUG
static bool testInvariants(Map *map) {
	if (!roadMapTestCount(map->roads)) {
		return false;
	}
	{
		bool success, *trunks = whichTrunks(map);
		success = roadMapTestTrunk(map->roads, trunks);
		free(trunks);
		if (!success) {
			return false;
		}
	}
	if (cityMapTest(map->cities, map->trie) == false)
		return false;
	{
		for (size_t i = 0; i < ROUTE_LIMIT; ++i) {
			if (map->routes[i])
				if (!trunkTest(map->routes[i])) {
					return false;
				}
		}
	}
	return true;
}
#endif // NDEBUG

// debug function used only in assertions
#ifndef NDEBUG
static bool *whichTrunks(const Map *map) {
	bool *ans = calloc(ROUTE_LIMIT, sizeof(bool));
	assert(ans);
	for (size_t i = 0; i < ROUTE_LIMIT; ++i)
		ans[i] = (map->routes[i] != NULL);
	return ans;
}
#endif // NDEBUG
//! @endcond