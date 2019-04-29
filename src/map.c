#include <assert.h>
#include <string.h>
#include "city.h"
#include "map.h"
#include "queue.h"
#include "trunk.h"
#include "trie.h"

#define ROUTE_ID_MAX 999

struct Map {
	CityMap *cities;
	Trunk *routes[1 + ROUTE_ID_MAX];
	Trie *v;
};

// auxiliary function declarations
bool checkRoad(const City *city1, const City *city2, unsigned length, int builtYear);

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
	for (int i = 0; i <= ROUTE_ID_MAX; ++i) {
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
	return roadInit(map->cities, map->v, info, map->routes);
}

bool repairRoad(Map *map, const char *city1, const char *city2, int repairYear) {
	bool b;
	City *c1, *c2;
	Road *r;
	c1 = trieFind(map->v, city1);
	c2 = trieFind(map->v, city2);
	b = c1 && c2;
	if (!b)
		return false;
	if ((r = roadFind(c1, c2)) == NULL)
		return false;
	return roadUpdate(r, repairYear);
}

bool newRoute(Map *map, unsigned routeId, const char *city1, const char *city2) {
	City *c1, *c2;
	Trunk *route;
	c1 = trieFind(map->v, city1);
	c2 = trieFind(map->v, city2);
	if (!c1 || !c2 || routeId > ROUTE_ID_MAX || map->routes[routeId] != NULL)
		return false;
	route = trunkBuild(c1, c2, map->cities, routeId);
	if (route == NULL)
		return false;
	map->routes[routeId] = route;
	trunkAttach(route);
	return true;
}

bool extendRoute(Map *map, unsigned routeId, const char *city) {
	bool ans = false;
	City *c;
	Trunk *extension, *route;
	unsigned *lengths;
	if (routeId > 999)
		return false;
	c = trieFind(map->v, city);
	route = map->routes[routeId];
	if (trunkHasCity(route, c))
		return false;
	lengths = trunkBlock(route);
	if (lengths != NULL) {
		extension = trunkExtend(map->cities, route, c);
		if (extension != NULL) {
			map->routes[routeId] = extension;
			ans = true;
		}
		trunkUnblock(route, lengths);
		free(route);
		free(lengths);
	}
	return ans;
}

bool removeRoad(Map *map, const char *city1, const char *city2) {
	City *c1 = trieFind(map->v, city1), *c2 = trieFind(map->v, city2);
	Road *r;
	if (!c1 || !c2)
		return false;
	r = roadFind(c1, c2);
	if (!r)
		return false;
	return roadDestroy(map->cities, r, map->routes);
}

const char *getRouteDescription(Map *map, unsigned routeId) {
	Trunk *route = map->routes[routeId];
	const char *ans = trunkDescription(route);
	return ans;
}

// auxiliary function definitions
bool checkRoad(const City *city1, const City *city2, unsigned length, int builtYear) {
	if (builtYear == 0 || length == 0 || city1 == city2)
		return false;
	else
		return !roadFind(city1, city2);
}
