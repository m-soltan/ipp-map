#include "city.h"
#include "city_map.h"
#include "map.h"
#include "queue.h"
#include "road.h"
#include "trunk.h"
#include "trie.h"

struct Map {
	CityMap *cities;
	RoadMap *roads;
	Trunk *routes[ROUTE_LIMIT];
	Trie *trie;
};

static bool addFromList(Map *map, NameList list, const int *years, const unsigned *roadLengths);
static bool correctRoute(unsigned routeId, const char *name1, const char *name2);
static bool destroyRoad(Map *map, Road *road);
static bool invalidId(unsigned routeId);
static bool namesAreCorrect(const char *city1, const char *city2);
static bool nameError(const char *str);
static bool testExistingRoads(Map *map, const char **names, const unsigned *roadLengths, size_t length);
static bool testInvariants(Map *map);
static bool testNameUniqueness(const char **names, size_t length);
static bool testRoute(Map *map, const char **names, const int *years, const unsigned *roadLengths, size_t length);
static bool testYears(Map *map, const char **names, const int *years, size_t length);
static bool *whichTrunks(const Map *map);
static void destroyTrunks(Map *map);

static void repairFromList(Map *map, NameList list, const int *years);
static Road *find(Trie *trie, const char *city1, const char *city2);

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
			ans = cityMapLoneRoad(map->cities, map->trie, info);
	}
	assert(testInvariants(map));
	if (ans && c1)
		assert(cityGetRoadCount(c1) == count1 + 1);
	if (ans && c2)
		assert(cityGetRoadCount(c2) == count2 + 1);
	return ans;
}

bool repairRoad(Map *map, const char *city1, const char *city2, int repairYear) {
	bool ans;
	City *c1, *c2;
	Road *r;
	c1 = trieFind(map->trie, city1);
	c2 = trieFind(map->trie, city2);
	if (!c1 || !c2)
		return false;
	r = roadFind(c1, c2);
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
		if (route && trunkGetLength(route) < SIZE_MAX) {
			assert(routeId < ROUTE_LIMIT);
			map->routes[routeId] = route;
			trunkAttach(route);
			assert(trunkTest(route));
			assert(testInvariants(map));
			return true;
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
	City *c1 = trieFind(map->trie, city1), *c2 = trieFind(map->trie, city2);
	Road *r = roadFind(c1, c2);
	if (r == NULL)
		return false;
	ans = destroyRoad(map, r);
	assert(!map->routes[410] || trunkTest(map->routes[410]));
	if (ans)
		assert(roadFind(c1, c2) == NULL);
	assert(testInvariants(map));
	return ans;
}

char *routeDescriptionAux(Map *map, unsigned routeId) {
	char *ans = malloc(sizeof(char));
	if (ans) {
		*ans = '\0';
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

void destroyTrunk(Trunk *trunks[ROUTE_LIMIT], unsigned trunkId) {
	trunkDestroy(&trunks[trunkId]);
}

bool routeFromList(Map *map, unsigned id, const char **names, unsigned *rLengths, int *years, size_t length) {
	if (!testRoute(map, names, years, rLengths, length))
		return false;
	const size_t cityCount = cityMapGetLength(map->cities);
	const size_t roadCount = roadMapGetLength(map->roads);
	NameList list = (NameList) {.length = length, .v  = names};
	bool addSuccess = addFromList(map, list, years, rLengths);
	if (addSuccess) {
		City *first, *last;
		first = trieFind(map->trie, names[0]);
		if (first == NULL) {
			first = cityMapGetAt(map->cities, roadCount);
		}
		last = trieFind(map->trie, names[length - 1]);
		if (last == NULL) {
			last = cityMapGetAt(map->cities, cityMapGetLength(map->cities) - 1);
		}
		size_t trunkLength = roadMapGetLength(map->roads) - roadCount;
		Road *const *roads = roadMapGetSuffix(map->roads, roadCount);
		Trunk *trunk = trunkMake(id, first, last, roads, trunkLength);
		if (trunk) {
			bool insertSuccess;
			City *const *suffix = cityMapSuffix(map->cities, cityCount);
			insertSuccess = trieAddFromList(map->trie, list, suffix);
			if (insertSuccess) {
				map->routes[id] = trunk;
				repairFromList(map, list, years);
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

static bool destroyRoad(Map *map, Road *road) {
	City *city1, *city2;
	roadGetCities(road, &city1, &city2);
	if (roadRouteCount(road) > 0) {
		bool moveSuccess = roadMoveTrunks(map->cities, map->routes, road);
		assert(testInvariants(map));
		if (!moveSuccess)
			return false;
	}
//	todo: remove
//	roadDisconnect(road);
	Trunk *trunk = map->routes[410];
	roadDetach(road, city1);
	assert(!trunk || trunkTest(trunk));
	roadDetach(road, city2);
	assert(!trunk || trunkTest(trunk));
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
	size_t i = 0;
	Trie *seen = trieInit();
	City *blank = cityDecoy();
	if (seen) {
		for (; i < length; ++i) {
			bool insertSuccess;
			if (trieFind(seen, names[i]))
				break;
			insertSuccess = trieInsert(seen, names[i], blank);
			if (!insertSuccess)
				break;
		}
		trieDestroy(&seen);
	}
	return i == length;
}

static bool testExistingRoads(Map *map, const char **names, const unsigned *roadLengths, size_t length) {
	for (size_t i = 1; i < length; ++i) {
		City *city1, *city2;
		city1 = trieFind(map->trie, names[i - 1]);
		city2 = trieFind(map->trie, names[i]);
		if (city1 && city2) {
			Road *road = roadFind(city1, city2);
			if (road && (roadGetLength(road) != roadLengths[i - 1]))
				return false;
		}
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
		Road *road = roadFind(city1, city2);
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
	for (size_t i = 0; i < list.length - 1; ++i) {
		const char *city1, *city2;
		city1 = list.v[i];
		city2 = list.v[i + 1];
		Road *road = find(map->trie, city1, city2);
		if (road == NULL) {
			bool success = addRoad(map, city1, city2, roadLengths[i], years[i]);
			if (!success)
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
		return roadFind(c1, c2);
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
		repairRoad(map, city1, city2, years[i]);
	}
}

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
	{
		for (size_t i = 0; i < ROUTE_LIMIT; ++i) {
			if (map->routes[i])
				if (!trunkTest(map->routes[i]))
					return false;
		}
	}
	return true;
}

static bool *whichTrunks(const Map *map) {
	bool *ans = calloc(ROUTE_LIMIT, sizeof(bool));
	assert(ans);
	for (size_t i = 0; i < ROUTE_LIMIT; ++i)
		ans[i] = (map->routes[i] != NULL);
	return ans;
}
