#include "city.h"
#include "map.h"
#include "road.h"
#include "trie.h"
#include "trunk.h"

#define ROAD_MAP_LENGTH 32

struct Road {
	City *city1, *city2;
	int year;
	unsigned length, routeCount;
	bool *routes;
};

struct RoadMap {
	Road **roads;
	size_t length, maxLength;
};

static bool add(RoadMap *roadMap, Road *road);
static bool adjust(RoadMap *roadMap);
static bool checkDestroyed(const Road *road, Trunk *trunks[ROUTE_LIMIT]);
static bool testCount(const Road *road);
static void removeLast(RoadMap *roadMap);
static Road *roadInit(RoadMap *roadMap);

bool roadReserve(Road *road) {
	if (road->routes == NULL) {
		road->routes = calloc(ROUTE_LIMIT, sizeof(bool));
		return road->routes != NULL;
	}
	return true;
}

unsigned roadGetLength(const Road *road) {
	return road->length;
}

unsigned roadRouteCount(const Road *road) {
	return road->routeCount;
}

bool cityMapLoneRoad(CityMap *cityMap, Trie *trie, RoadInfo roadInfo) {
	const char *names[2];
	City *cities[2];
	names[0] = roadInfo.city1;
	names[1] = roadInfo.city2;
	NameList list = (NameList) {.length = 2, .v = names};
	Road *road = roadInit(roadInfo.roadMap);
	if (!road)
		return false;
	cities[0] = cityAdd(cityMap, roadInfo.city1, road);
	if (cities[0]) {
		cities[1] = cityAdd(cityMap, roadInfo.city2, road);
		if (cities[1]) {
			bool successInit = roadInitFields(road, roadInfo, cities[0], cities[1]);
			if (successInit) {
				bool successAdd = trieAddFromList(trie, list, cities);
				if (successAdd)
					return true;
				roadFree(&road);
			}
			free(cities[1]);
		}
		free(cities[0]);
	}
	free(road);
	return false;
}

bool roadInitFields(Road *road, RoadInfo info, City *city1, City *city2) {
	if (city1 > city2)
		return roadInitFields(road, info, city2, city1);
	*road = (Road) {
		.city1 = city1,
		.city2 = city2,
		.year = info.builtYear,
		.length = info.length,
		.routeCount = 0,
		.routes = calloc(ROUTE_LIMIT, sizeof(bool))
	};
	if (!road->routes)
		return false;
	return true;
}

City *roadIntersect(Road *road1, Road *road2) {
	assert(road1 != road2);
	if (roadHasIntersection(road1, road2)) {
		City *city1 = road1->city1, *city2 = road1->city2;
		if (roadHasCity(road2, city1))
			return city1;
		else
			return city2;
	} else {
		return NULL;
	}
}

void roadGetCities(Road *road, City **city1, City **city2) {
	assert(city1 && city2);
	*city1 = road->city1;
	*city2 = road->city2;
}

void roadTrunkRemove(Road *road, unsigned trunkId) {
	assert(road->routes && road->routes[trunkId]);
	road->routes[trunkId] = false;
	--road->routeCount;
}

void roadFree(Road **pRoad) {
	Road *road = *pRoad;
	assert(!roadFind(road->city1, road->city2));
	*pRoad = NULL;
	free(road->routes);
	free(road);
}

bool roadExtend(CityMap *m, Trie *t, City *city, RoadInfo info) {
	bool successAdd, successInsert;
	const char *str = (info.city1 ? info.city1 : info.city2);
	assert(str);
	Road *road = roadInit(info.roadMap);
	if (road) {
		City *newCity = cityAdd(m, str, road);
		if (newCity) {
			successAdd = cityConnectRoad(city, road);
			if (successAdd) {
				successInsert = trieInsert(t, str, newCity);
				if (successInsert) {
					roadInitFields(road, info, city, newCity);
					return true;
				}
				cityDetach(city, road);
			}
			cityDestroy(&newCity);
		}
		free(road);
	}
	return false;
}

bool roadLink(RoadMap *roadMap, City *city1, City *city2, unsigned length, int year) {
	if (roadFind(city1, city2) != NULL)
		return false;
	Road *r = roadInit(roadMap);
	if (r) {
		if (cityMakeRoad(city1, city2, r)) {
			*r = (Road) {.length = length, .year = year};
			r->city1 = (city1 < city2 ? city1 : city2);
			r->city2 = (city1 < city2 ? city2 : city1);
			return true;
		}
		free(r);
	}
	return false;
}

int roadGetYear(const Road *road) {
	return road->year;
}

bool roadUpdate(Road *road, int year) {
	bool ans = year != 0 && year >= road->year;
	if (ans)
		road->year = year;
	return ans;
}

void roadDestroyTrunks(Trunk *trunks[ROUTE_LIMIT], Road *road) {
	for (size_t i = 0; i< ROUTE_LIMIT; ++i) {
		if (road->routes[i])
			trunkFree(&trunks[i]);
	}
	road->routeCount = 0;
	assert(checkDestroyed(road, trunks));
	free(road->routes);
	road->routes = NULL;
}

int roadWrite(char *str, const Road *road, const City *city) {
	int ans;
	ans = sprintf(str, ";%u;%i;", road->length, road->year);
	ans += cityCopyName(str + ans, city);
	assert(ans > 0);
	return ans;
}

void roadTrunkAdd(Road *road, unsigned trunkId) {
	assert(road->routes);
	if (road->routes[trunkId])
		return;
	++road->routeCount;
	road->routes[trunkId] = true;
}

void roadUnblock(Road *road, unsigned length) {
	assert(roadGetLength(road) == (unsigned) -1);
	cityBlock(road->city1);
	cityBlock(road->city2);
	road->length = length;
}

void roadDetach(Road *road, const City *city) {
	if (road->city1 == city) {
		cityDetach(road->city1, road);
		road->city1 = NULL;
	} else if (road->city2 == city) {
		cityDetach(road->city2, road);
		road->city2 = NULL;
	} else {
		assert(false);
	}
}

bool roadHasCity(const Road *road, const City *city) {
	assert(city != NULL);
	if (city == road->city1)
		return true;
	else if (city == road->city2)
		return true;
	else
		return false;
}

unsigned roadBlock(Road *road) {
	cityUnblock(road->city1);
	cityUnblock(road->city2);
	unsigned ans = road->length;
	road->length = (unsigned) -1;
	return ans;
}

bool roadMoveTrunks(CityMap *cityMap, Trunk *trunks[ROUTE_LIMIT], Road *road) {
	Trunk **replacements = rebuildTrunks(cityMap, road, trunks);
	const unsigned routeCount = roadRouteCount(road);
	if (!replacements)
		return false;
	roadDestroyTrunks(trunks, road);
	for (size_t i = 0; i < routeCount; ++i) {
		trunkAttach(replacements[i]);
	}
	for (size_t i = 0; i < routeCount; ++i) {
		Trunk **trunk = &trunks[trunkGetId(replacements[i])];
		assert(*trunk == NULL);
		*trunk = replacements[i];
	}
	free(replacements);
	return true;
}

void roadDisconnect(Road *road) {
	roadDetach(road, road->city1);
	roadDetach(road, road->city2);
}

RoadMap *roadMapInit(void) {
	RoadMap *ans = malloc(sizeof(RoadMap));
	if (ans) {
		*ans = (RoadMap) {
			.length = 0,
			.maxLength = ROAD_MAP_LENGTH,
			.roads = calloc(ROAD_MAP_LENGTH, sizeof(Road *)),
		};
		if (ans->roads)
			return ans;
		free(ans);
	}
	return NULL;
}

size_t roadMapGetLength(const RoadMap *roadMap) {
	return roadMap->length;
}

void roadMapDestroy(RoadMap **pRoadMap) {
	RoadMap *temp = *pRoadMap;
	for (size_t i = 0; i < temp->length; ++i) {
		Road *road = temp->roads[i];
		temp->roads[i] = NULL;
		if (road->routes != NULL) {
			free(road->routes);
			road->routes = NULL;
		}
		free(road);
	}
	free(temp->roads);
	free(*pRoadMap);
	*pRoadMap = NULL;
}

void roadMapTrim(RoadMap *roadMap, size_t length) {
	if (roadMap->length < length)
		assert(false);
	const size_t n = roadMap->length - length;
	for (size_t i = 0; i < n; ++i)
		removeLast(roadMap);
}

Road *const *roadMapGetSuffix(RoadMap *roadMap, size_t start) {
	return &roadMap->roads[start];
}

bool roadMapTestTrunk(const RoadMap *roadMap, const bool *trunks) {
	for (size_t i = 0; i < roadMap->length; ++i) {
		Road *road = roadMap->roads[i];
		if (road->routes == NULL)
			continue;
		for (size_t j = 0; j < ROUTE_LIMIT; ++j) {
			if (road->routes[j] && !trunks[j]) {
				return false;
			}
		}
	}
	return true;
}

bool roadHasRoute(const Road *road, unsigned routeId) {
	return road->routes[routeId];
}

bool roadMapTestCount(const RoadMap *roadMap) {
	for (size_t i = 0; i < roadMap->length; ++i) {
		if (!testCount(roadMap->roads[i])) {
			return false;
		}
	}
	return true;
}

void roadGetIds(const Road *road, unsigned *result) {
	for (size_t i = 0, j = 0; i < ROUTE_LIMIT; ++i) {
		assert(i < ROUTE_LIMIT);
		if (roadHasRoute(road, i)) {
			result[j] = i;
			++j;
		} else {
			result[ROUTE_LIMIT - i + j - 1] = 0;
		}
	}
}

bool roadHasIntersection(const Road *road1, const Road *road2) {
	assert(road1 != road2);
	const City *city1 = road1->city1, *city2 = road1->city2;
	return roadHasCity(road2, city1) || roadHasCity(road2, city2);
}

static bool adjust(RoadMap *roadMap) {
	assert(roadMap);
	size_t newMax = 2 * roadMap->maxLength;
	Road **temp = realloc(roadMap->roads, newMax * sizeof(Road *));
	if (temp) {
		roadMap->maxLength = newMax;
		roadMap->roads = temp;
		return true;
	}
	return false;
}

static bool add(RoadMap *roadMap, Road *road) {
	size_t length = roadMap->length;

	if (length == roadMap->maxLength) {
		bool adjustSuccess = adjust(roadMap);
		if (!adjustSuccess)
			return false;
	}
	assert(length < roadMap->maxLength);

	roadMap->roads[length] = road;
	roadMap->length = length + 1;
	return true;
}

static Road *roadInit(RoadMap *roadMap) {
	Road *ans = malloc(sizeof(Road));
	if (ans) {
		bool addSuccess = add(roadMap, ans);
		if (addSuccess)
			return ans;
		free(ans);
	}
	return NULL;
}

static void removeLast(RoadMap *roadMap) {
	Road **pLast = &roadMap->roads[roadMap->length - 1];
	Road *last = *pLast;
	assert(roadRouteCount(last) == 0);
	if (last->routes) {
		free(last->routes);
		last->routes = NULL;
	}
	free(last);
	*pLast = NULL;
	--roadMap->length;
}

bool checkDestroyed(const Road *road, Trunk *trunks[ROUTE_LIMIT]) {
	for (size_t i = 0; i < ROUTE_LIMIT; ++i) {
		if (roadHasRoute(road, i) && trunks[i])
			return false;
	}
	return true;
}

static bool testCount(const Road *road) {
	for (size_t i = 1, count = 0; true; ++i) {
		if (count == roadRouteCount(road))
			return true;
		if (i == ROUTE_LIMIT)
			return false;
		if (roadHasRoute(road, i))
			++count;
	}
}