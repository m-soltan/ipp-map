#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "city.h"
#include "city_map.h"
#include "road.h"
#include "trie.h"
#include "trunk.h"

typedef struct Detour Detour;

struct CityMap {
	size_t length, lengthMax;
	City **cities;
};

static bool insertRoad(CityMap *cityMap, Trie *trie, Road **road);
static Trie **insertCity(Trie *trie, City **city, bool *success);
static Trunk **rebuildTrunks(CityMap *cityMap, Road *road, Trunk *trunks[1000]);

// linked function definitions
CityMap *cityMapInit() {
	CityMap *ans = malloc(sizeof(CityMap));
	if (ans) {
		*ans = (CityMap) {.length = 0, .lengthMax = 8};
		ans->cities = malloc(ans->lengthMax * sizeof(City *));
		if (ans->cities)
			return ans;
		free(ans);
	}
	return NULL;
}

size_t cityMapGetLength(const CityMap *cityMap) {
	return cityMap->length;
}

void cityMapDestroy(CityMap **pCityMap) {
	CityMap *temp = *pCityMap;
	while (temp->length > 0) {
		City **pCity = &temp->cities[temp->length - 1];
		while (cityGetRoadCount(*pCity) > 0) {
			cityDetachLast(*pCity);
		}
		cityDestroy(pCity, temp);
	}
	free((*pCityMap)->cities);
	free(*pCityMap);
	*pCityMap = NULL;
}

bool roadAdjust(const City *from, const City *to, size_t length) {
	Road *road = roadFind(from, to);
	return roadReserve(road, length);
}

// TODO: rebuilding the routes broken by the removal
// TODO: just connect the separated parts, leave the rest of the route
bool roadDestroy(CityMap *cityMap, Road *road, Trunk *trunks[1000]) {
	if (road->routes != NULL) {
		assert(trunks != NULL);
		Trunk **replacements;
		replacements = rebuildTrunks(cityMap, road, trunks);
		if (replacements) {
			for (size_t i = 0; i < road->routeCount; ++i) {
				trunkFree(&trunks[road->routes[i]]);
				assert(false); //TODO
			}
		}
		return false;
	}
	cityDetach(road->city1, road);
	cityDetach(road->city2, road);
	return true;
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
					road->length = info.length;
					road->year = info.builtYear;
					road->city1 = (newCity < city ? newCity : city);
					road->city2 = (newCity < city ? city : newCity);
					road->routes = NULL;
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

bool roadInit(CityMap *m, Trie *t, RoadInfo info) {
	Road *ans = malloc(sizeof(Road));
	if (!ans)
		return NULL;
	City *c1 = cityInit(m, info.city1, ans), *c2 = cityInit(m, info.city2, ans);
	if (c1 && c2) {
		*ans = (Road) {.length = info.length, .year = info.builtYear};
		ans->routeCount = 0;
		ans->routeMax = 0;
		ans->city1 = (c1 < c2 ? c1 : c2);
		ans->city2 = (c1 < c2 ? c2 : c1);
	} else {
		free(ans);
		if (c1)
			free(c1);
		if (c2)
			free(c2);
		return NULL;
	}
	return insertRoad(m, t, &ans);
}

bool roadUpdate(Road *road, int year) {
	bool ans = year != 0 && year >= road->year;
	if (ans)
		road->year = year;
	return ans;
}

int roadGetYear(const Road *road) {
	return road->year;
}

Road * roadFind(const City *city1, const City *city2) {
	if (city1 == city2)
		return NULL;
	if (city1 > city2)
		return roadFind(city2, city1);
	return cityRoadTo(city1, city2);
}

size_t roadWrite(char *str, const Road *road, const City *city) {
	int ans;
	ans = sprintf(str, ";%u;%i;", road->length, road->year);
	ans += cityGetName(str + ans, city);
	assert(ans > 0);
	return (size_t) ans;
}

void roadTrunkAdd(Road *r, size_t trunk) {
	assert(r->routeCount < r->routeMax);
	r->routes[r->routeCount] = trunk;
	++r->routeCount;
}

void roadUnblock(Road *road, unsigned length) {
	assert(road->length == (unsigned) -1);
	cityBlock(road->city1);
	cityUnblock(road->city2);
	road->length = length;
}

void roadGetCities(Road *road, City **city1, City **city2) {
	assert(city1 && city2);
	*city1 = road->city1;
	*city2 = road->city2;
}

unsigned roadBlock(Road *road) {
	cityUnblock(road->city1);
	cityUnblock(road->city2);
	unsigned ans = road->length;
	road->length = (unsigned) -1;
	return ans;
}

City **cityMapAdd(CityMap *cityMap) {
	City **ans, **temp;
	size_t size;
	if (cityMap->length == cityMap->lengthMax) {
		assert(cityMap->lengthMax);
		size = 2 * cityMap->lengthMax;
		temp = realloc(cityMap->cities, size * sizeof(City *));
		if (temp == NULL)
			return NULL;
		cityMap->cities = temp;
		cityMap->lengthMax *= 2;
	}
	ans = &cityMap->cities[cityMap->length - 1];
	++cityMap->length;
	return ans;

}

void cityMapRemove(CityMap *cityMap) {
	--cityMap->length;
}

bool cityMapIsLast(const CityMap *cityMap, City *const *city) {
	City **last = &cityMap->cities[cityMap->length - 1];
	return city == last;
}

// auxiliary function definitions
static bool insertRoad(CityMap *cityMap, Trie *trie, Road **road) {
	bool success1, success2;
	City **city1 = &(*road)->city1, **city2 = &(*road)->city2;
	Trie **parent;
	parent = insertCity(trie, city1, &success1);
	if (success1) {
		insertCity(trie, city2, &success2);
		if (success2)
			return true;
		if (parent)
			trieDestroy(parent);
	}
	roadDestroy(cityMap, *road, NULL);
	return false;
}

static Trie **insertCity(Trie *trie, City **city, bool *success) {
	Trie **ans = NULL;
	assert(success);
	char *name = malloc(sizeof(char) * cityGetNameLength(*city));
	if (name) {
		cityGetName(name, *city);
		ans = trieInsert(trie, name, city, success);
		free(name);
	}
	return ans;
}

// TODO: insert a detour into each trunk
static Trunk **rebuildTrunks(CityMap *cityMap, Road *road, Trunk *trunks[1000]) {
	Trunk **replacements = malloc(road->routeCount * sizeof(Trunk *));
	if (replacements == NULL)
		return false;
	for (size_t i = 0; i < road->routeCount; ++i) {
		Trunk *trunk = trunks[road->routes[i]];
		replacements[i] = trunkDetour(cityMap, trunk, road);
		if (replacements[i] == NULL) {
			for (size_t j = 0; j < i; ++j)
				trunkFree(&replacements[i]);
			free(replacements);
			return false;
		}

	}
	return replacements;
}
