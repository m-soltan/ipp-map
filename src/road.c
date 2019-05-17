#include "city.h"
#include "map.h"
#include "road.h"
#include "trie.h"
#include "trunk.h"

struct Road {
	City *city1, *city2;
	int year;
	unsigned length, routeCount, routeMax;
	unsigned *routes;
};

static bool initTrunks(Road *road, unsigned length);
static bool insertRoad(Trie *trie, Road **pRoad);
static Trie **insertCity(Trie *trie, City **city, bool *success);

bool roadReserve(Road *road, unsigned length) {
	assert(length <= ROUTE_LIMIT);
	if (road->routes == NULL)
		return initTrunks(road, length);
	if (length > road->routeMax) {
		unsigned newMax = road->routeMax, *tmp;
		while (newMax < length)
			newMax *= 2;
		tmp = realloc(road->routes, newMax * sizeof(unsigned));
		if (tmp == NULL)
			return false;
		road->routes = tmp;
	}
	return true;
}

unsigned roadGetFree(const Road *road) {
	return road->routeMax - road->routeCount;
}

unsigned roadGetLength(const Road *road) {
	return road->length;
}

unsigned roadRouteCount(const Road *road) {
	return road->routeCount;
}

City *roadGetOther(Road *road, City *city) {
	if (road->city1 == city) {
		return road->city2;
	} else {
		return road->city1;
	}
}

bool cityMapLoneRoad(CityMap *m, Trie *t, RoadInfo info) {
	Road *road = malloc(sizeof(Road));
	if (!road)
		return false;
	City *c1 = cityInit(m, info.city1, road);
	if (c1) {
		City *c2 = cityInit(m, info.city2, road);
		if (c2) {
			bool successInit = roadInitFields(road, info, c1, c2);
			if (successInit) {
				if (insertRoad(t, &road)) {
					return true;
				}
				roadFree(&road);
			}
			free(c2);
		}
		free(c1);
	}
	free(road);
	return false;
}

bool roadInitFields(Road *road, RoadInfo info, City *city1, City *city2) {
	unsigned initialMax = ROUTE_LIMIT;
	if (city1 > city2)
		return roadInitFields(road, info, city2, city1);
	*road = (Road) {
		.city1 = city1,
		.city2 = city2,
		.year = info.builtYear,
		.length = info.length,
		.routeCount = 0,
		.routeMax = initialMax,
		.routes = malloc(initialMax * sizeof(Trunk *))
	};
	if (!road->routes)
		return false;
	return true;
}

bool roadIntersect(Road *road1, Road *road2) {
	City *cities[2][2];
	roadGetCities(road1, &cities[0][0], &cities[0][1]);
	roadGetCities(road2, &cities[1][0], &cities[1][1]);
	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 2; ++j)
			if (cities[0][i] == cities[0][j])
				return true;
	return false;
}

void roadGetCities(Road *road, City **city1, City **city2) {
	assert(city1 && city2);
	*city1 = road->city1;
	*city2 = road->city2;
}

void roadTrunkRemove(Road *road, unsigned trunkId) {
	unsigned *last = &road->routes[road->routeCount - 1];
	for (unsigned i = 0; i < road->routeCount; ++i)
		if (road->routes[i] == trunkId) {
			road->routes[i] = *last;
			return;
		}
	assert(false);
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
	Road *road = malloc(sizeof(Road));
	if (road) {
		City *newCity = cityInit(m, str, road);
		if (newCity) {
			successAdd = cityConnectRoad(city, road);
			if (successAdd) {
//				TODO
				trieInsert(t, str, &newCity, &successInsert);
				if (successInsert) {
					roadInitFields(road, info, city, newCity);
					return true;
				}
				cityDetach(city, road);
			}
			cityDestroy(&newCity, m);
		}
		free(road);
	}
	return false;
}

bool roadLink(City *city1, City *city2, unsigned length, int year) {
	if (roadFind(city1, city2) != NULL)
		return false;
	Road *r = malloc(sizeof(Road));
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
	for (size_t i = 0; i < road->routeCount; ++i)
		destroyTrunk(trunks, road->routes[i]);
}

int roadWrite(char *str, const Road *road, const City *city) {
	int ans;
	ans = sprintf(str, ";%u;%i;", road->length, road->year);
	ans += cityGetName(str + ans, city);
	assert(ans > 0);
	return ans;
}

void roadTrunkAdd(Road *r, unsigned trunkId) {
	r->routes[r->routeCount] = trunkId;
	++r->routeCount;
}

void roadUnblock(Road *road, unsigned length) {
	assert(roadGetLength(road) == (unsigned) -1);
	cityBlock(road->city1);
	cityBlock(road->city2);
	road->length = length;
}

const unsigned *roadGetRoutes(const Road *road) {
	return road->routes;
}

void roadDetach(Road *road, const City *city) {
	if (road->city1 == city) {
		cityDetach(road->city1, road);
		road->city1 = NULL;
	} else {
		assert(road->city2 == city);
		cityDetach(road->city2, road);
		road->city2 = NULL;
	}
	if (road->city1 == NULL && road->city2 == NULL) {
		free(road->routes);
		free(road);
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
	for (size_t i = 0; i < routeCount; ++i)
		trunkAttach(replacements[i]);
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

Road **roadGuardian(City *city1, City *city2) {
	Road **ans = malloc(sizeof(Road *));
	if (ans != NULL) {
		*ans = malloc(sizeof(Road));
		if (*ans != NULL) {
			**ans = (Road) {
				.city1 = city1,
				.city2 = city2,
				.year = INT16_MAX,
				.length = (unsigned) -1,
				.routeCount = 0,
				.routeMax = ROUTE_LIMIT,
			};
			(*ans)->routes = malloc((*ans)->routeMax * sizeof(Trunk *));
			if ((*ans)->routes)
				return ans;
			free(*ans);
		}
		free(ans);
	}
	return NULL;
}


// auxiliary function definitons
static bool initTrunks(Road *road, unsigned length) {
	assert(length > 0);
	road->routes = malloc(length * sizeof(unsigned));
	if (road->routes) {
		road->routeMax = length;
		return true;
	}
	return false;
}

static bool insertRoad(Trie *trie, Road **pRoad) {
	bool success1, success2;
	Road *road = *pRoad;
	Trie **parent;
	parent = insertCity(trie, &road->city1, &success1);
	if (success1) {
		insertCity(trie, &road->city2, &success2);
		if (success2)
			return true;
		if (parent)
			trieDestroy(parent);
	}
	roadFree(pRoad);
	return false;
}

static Trie **insertCity(Trie *trie, City **city, bool *success) {
	Trie **ans = NULL;
	assert(success);
	char *name = malloc(cityGetNameLength(*city) + 1);
	if (name) {
		cityGetName(name, *city);
		ans = trieInsert(trie, name, city, success);
		free(name);
	}
	return ans;
}

