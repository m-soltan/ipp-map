#include "city.h"
#include "city_map.h"
#include "map.h"
#include "queue.h"
#include "road.h"
#include "trunk.h"
#include "trie.h"

struct Map {
	CityMap *cities;
	Trunk *routes[ROUTE_LIMIT];
	Trie *v;
};

static bool destroyRoad(Map *map, Road *road);

// linked function definitions
Map *newMap(void) {
	Map *ans = malloc(sizeof(Map));
	if (ans) {
		*ans = (Map) {.v = trieInit()};
		if (ans->v) {
			ans->cities = cityMapInit();
			if (ans->cities)
				return ans;
			free(ans->v);
		}
		free(ans);
	}
	return NULL;
}

void deleteMap(Map *map) {
	if (map == NULL)
		return;
	cityMapDestroy(&map->cities);
	trieDestroy(&map->v);
	for (int i = 0; i < ROUTE_LIMIT; ++i) {
		if (map->routes[i] != NULL)
			trunkFree(&map->routes[i]);
	}
	free(map);
}

bool addRoad(Map *map, const char *city1, const char *city2, unsigned length, int builtYear) {
	if (builtYear == 0 || length == 0 || strcmp(city1, city2) == 0)
		return false;
	RoadInfo info = (RoadInfo) {.builtYear = builtYear, .length = length};
	City *c1 = trieFind(map->v, city1), *c2 = trieFind(map->v, city2);
	if (c1 && c2)
		return roadLink(c1, c2, length, builtYear);
	info.city1 = (c1 ? NULL : city1);
	info.city2 = (c2 ? NULL : city2);
	if (c1 || c2)
		return roadExtend(map->cities, map->v, (c1 != NULL ? c1 : c2), info);
	return cityMapLoneRoad(map->cities, map->v, info);
}

bool repairRoad(Map *map, const char *city1, const char *city2, int repairYear) {
	City *c1, *c2;
	Road *r;
	c1 = trieFind(map->v, city1);
	c2 = trieFind(map->v, city2);
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
	c1 = trieFind(map->v, city1);
	c2 = trieFind(map->v, city2);
	if (c1 && c2 && routeId < ROUTE_LIMIT && map->routes[routeId] == NULL) {
		route = trunkBuild(c1, c2, map->cities, routeId);
		if (route) {
			map->routes[routeId] = route;
			trunkAttach(route);
			return true;
		}
	}
	return false;
}

bool extendRoute(Map *map, unsigned routeId, const char *city) {
	bool ans = false;
	City *c;
	Trunk *extension, *route;
	if (routeId > 999)
		return false;
	c = trieFind(map->v, city);
	route = map->routes[routeId];
	if (trunkHasCity(route, c))
		return false;
	trunkBlock(route);
	extension = trunkExtend(map->cities, route, c);
	if (extension != NULL) {
		map->routes[routeId] = extension;
		ans = true;
	}
	trunkUnblock(route);
	free(route);
	return ans;
}

bool removeRoad(Map *map, const char *city1, const char *city2) {
	City *c1 = trieFind(map->v, city1), *c2 = trieFind(map->v, city2);
	Road *r;
	if (c1 == NULL || c2 == NULL)
		return false;
	r = roadFind(c1, c2);
	if (r == NULL)
		return false;
	return destroyRoad(map, r);
}

const char *getRouteDescription(Map *map, unsigned routeId) {
	Trunk *route = map->routes[routeId];
	const char *ans = trunkDescription(route);
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
