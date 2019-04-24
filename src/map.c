#include <assert.h>
#include <string.h>
#include "map.h"
#include "trie.h"

struct Map {
	Trie *v;
};

// auxiliary function declarations
bool checkRoad(const City *city1, const City *city2, unsigned length, int builtYear);
//bool makeRoad(City *city1, City *city2, unsigned length, int builtYear);

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

// TODO
bool addRoad(Map *map, const char *city1, const char *city2, unsigned length, int builtYear) {
	Road *r;
	if (builtYear == 0) return false;
	if (length == 0) return false;
	if (!strcmp(city1, city2)) return false;
	City *c1 = trieFind(map->v, city1), *c2 = trieFind(map->v, city2);
	if (c1 && c2)
		return roadLink(c1, c2);
	if (c1)
		return roadExtend(map->v, c1, city2);
	if (c2)
		return roadExtend(map->v, c2, city1);
	r = roadInit(city1, city2, length, builtYear);
	return roadInsert(map->v, &r);
}

// TODO
bool repairRoad(Map *map, const char *city1, const char *city2, int repairYear) {
	return 0;
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
// TODO
bool checkRoad(const City *city1, const City *city2, unsigned length, int builtYear) {
	if (builtYear == 0 || length == 0 || city1 == city2)
		return false;
	else
		return !roadFind(city1, city2);
}



// TODO
//bool makeRoad(City *city1, City *city2, unsigned length, int builtYear) {
//	if (builtYear == 0) return false;
//	if (length == 0) return false;
//	if (city1 == city2) return false;
//	if (roadFind(city1, city2)) return false;
//	Road *newRoad = roadInit(city1, city2, length, builtYear);
//	if (newRoad == NULL) return false;
//	cityAddRoad(city1, newRoad);
//	cityAddRoad(city2, newRoad);
//	return false;
//}
