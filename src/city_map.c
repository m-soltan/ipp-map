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

static bool empty(const CityMap *cityMap);
static void destroyLast(CityMap *cityMap);

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
	for (size_t i = temp->length; i > 0; --i) {
		assert(i == temp->length);
		destroyLast(*pCityMap);
	}
	free((*pCityMap)->cities);
	free(*pCityMap);
	*pCityMap = NULL;
}

bool roadAdjust(City *from, City *to, unsigned length) {
	Road *road = roadFind(from, to);
	return roadReserve(road, length);
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
	ans = &cityMap->cities[cityMap->length];
	++cityMap->length;
	return ans;

}

void cityMapRemove(CityMap *cityMap) {
	--cityMap->length;
}

bool cityMapIsLast(const CityMap *cityMap, City *const *city) {
	City **last = &cityMap->cities[cityMap->length - 1];
	return *city == *last;
}

// auxiliary function definitions
static void destroyLast(CityMap *cityMap) {
	assert(!empty(cityMap));
	City **last = &cityMap->cities[cityMap->length - 1];
	for (size_t i = cityGetRoadCount(*last); i > 0; --i) {
		assert(i == cityGetRoadCount(*last));
		cityDetachLast(*last);
	}
	cityDestroy(last, cityMap);
	--cityMap->length;
}

bool empty(const CityMap *cityMap) {
	return cityMap->length == 0;
}
