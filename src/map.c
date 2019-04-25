#include <assert.h>
#include <string.h>
#include "leftist.h"
#include "map.h"
#include "trunk.h"
#include "trie.h"

struct Map {
	Trunk *routes;
	Trie *v;
};

// auxiliary function declarations
bool checkRoad(const City *city1, const City *city2, unsigned length, int builtYear);

// linked function definitions
Map *newMap(void) {
	Map *ans = malloc(sizeof(Map));
	if (ans) {
		ans->v = trieInit();
		if (ans->v)
			return ans;
		free(ans);
	}
	return NULL;
}

// TODO
void deleteMap(Map *map) {

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
		return roadExtend(map->v, (c1 ? c1 : c2), info);
	return roadInit(map->v, info);
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

// TODO
bool newRoute(Map *map, unsigned routeId, const char *city1, const char *city2) {
	return 0;
}

// TODO
bool extendRoute(Map *map, unsigned routeId, const char *city) {
	return 0;
}

// TODO
bool removeRoad(Map *map, const char *city1, const char *city2) {
	
	return 0;
}

// TODO
const char *getRouteDescription(Map *map, unsigned routeId) {
	return NULL;
}

// auxiliary function definitions
bool checkRoad(const City *city1, const City *city2, unsigned length, int builtYear) {
	if (builtYear == 0 || length == 0 || city1 == city2)
		return false;
	else
		return !roadFind(city1, city2);
}
