#include "city.h"
#include "city_map.h"
#include "map.h"
#include "queue.h"
#include "road.h"
#include "trunk.h"
#include "trie.h"

typedef struct RoadList RoadList;

struct Map {
	CityMap *cities;
	RoadMap *roads;
	Trunk *routes[ROUTE_LIMIT];
	Trie *trie;
};

struct RoadList {
	int result;
	int pad;
	City *first, *last;
	Road *const *roads;
	size_t length;
};

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
static void repairFromList(Map *map, NameList list, const int *years);
static Road *find(Trie *trie, const char *city1, const char *city2);

Map *newMap(void) {
	Map *ans = malloc(sizeof(Map));
	if (ans) {
		*ans = (Map) {.trie = trieInit()};
		if (ans->trie) {
			ans->cities = cityMapInit();
			if (ans->cities) {
				ans->roads = roadMapInit();
				if (ans->roads)
					return ans;
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
	cityMapDestroy(&map->cities);
	trieDestroy(&map->trie);
	assert(map->routes[0] == NULL);
	for (size_t i = 1; i < ROUTE_LIMIT; ++i) {
		if (map->routes[i] != NULL)
			trunkFree(&map->routes[i]);
	}
	free(map);
}

bool addRoad(Map *map, const char *city1, const char *city2, unsigned length, int builtYear) {
	if (builtYear == 0 || length == 0 || !namesAreCorrect(city1, city2))
		return false;
	RoadInfo info = (RoadInfo) {
			.builtYear = builtYear,
			.length = length,
			.roadMap = map->roads,
	};
	City *c1 = trieFind(map->trie, city1), *c2 = trieFind(map->trie, city2);
	if (c1 && c2)
		return roadLink(map->roads, c1, c2, length, builtYear);
	info.city1 = (c1 ? NULL : city1);
	info.city2 = (c2 ? NULL : city2);
	if (c1 || c2)
		return roadExtend(map->cities, map->trie, (c1 != NULL ? c1 : c2), info);
	return cityMapLoneRoad(map->cities, map->trie, info);
}

bool repairRoad(Map *map, const char *city1, const char *city2, int repairYear) {
	City *c1, *c2;
	Road *r;
	c1 = trieFind(map->trie, city1);
	c2 = trieFind(map->trie, city2);
	if (!c1 || !c2)
		return false;
	r = roadFind(c1, c2);
	if (r == NULL)
		return false;
	return roadUpdate(r, repairYear);
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
			map->routes[routeId] = route;
			trunkAttach(route);
			return true;
		}
	}
	return false;
}

bool extendRoute(Map *map, unsigned routeId, const char *city) {
	City *c;
	Trunk *extension, *route;
	if (invalidId(routeId) || nameError(city))
		return false;
	c = trieFind(map->trie, city);
	route = map->routes[routeId];
	if (trunkHasCity(route, c))
		return false;
	trunkBlock(route);
	extension = trunkExtend(map->cities, route, c);
	trunkUnblock(route);
	if (extension == NULL)
		return false;
	map->routes[routeId] = extension;
	trunkFree(&route);
	return true;
}

bool removeRoad(Map *map, const char *city1, const char *city2) {
	City *c1 = trieFind(map->trie, city1), *c2 = trieFind(map->trie, city2);
	Road *r;
	if (c1 == NULL || c2 == NULL)
		return false;
	r = roadFind(c1, c2);
	if (r == NULL)
		return false;
	return destroyRoad(map, r);
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
	unsigned routeCount = roadRouteCount(road);
	assert(routeCount > 0);
	Trunk **replacements = malloc(routeCount * sizeof(Trunk *));
	if (replacements == NULL)
		return NULL;
	const unsigned *routeIds = roadGetRoutes(road);
	for (size_t i = 0; i < routeCount; ++i) {
		Trunk *trunk = trunks[routeIds[i]];
		replacements[i] = trunkAddDetour(cityMap, trunk, road);
		if (replacements[i] == NULL) {
			for (size_t j = 0; j < i; ++j)
				trunkFree(&replacements[i]);
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
	return 0;
}

static bool destroyRoad(Map *map, Road *road) {
	const unsigned routeCount = roadRouteCount(road);
	if (routeCount > 0) {
		bool moveSuccess = roadMoveTrunks(map->cities, map->routes, road);
		if (!moveSuccess)
			return false;
	}
	roadDisconnect(road);
	return true;
}

static bool namesAreCorrect(const char *city1, const char *city2) {
	bool ans = true;
	ans = ans && !nameError(city1);
	ans = ans &&!nameError(city2);
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

bool testNameUniqueness(const char **names, size_t length) {
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

bool testExistingRoads(Map *map, const char **names, const unsigned *roadLengths, size_t length) {
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

bool testYears(Map *map, const char **names, const int *years, size_t length) {
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

bool testRoute(Map *map, const char **names, const int *years, const unsigned *roadLengths, size_t length) {
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

Road *find(Trie *trie, const char *city1, const char *city2) {
	City *c1, *c2;
	c1 = trieFind(trie, city1);
	c2 = trieFind(trie, city2);
	if (c1 && c2)
		return roadFind(c1, c2);
	else
		return NULL;
}

void repairFromList(Map *map, NameList list, const int *years) {
	for (size_t i = 1; i < list.length; ++i) {
		const char *city1 = list.v[i - 1];
		const char *city2 = list.v[i];
		repairRoad(map, city1, city2, years[i]);
	}
}

