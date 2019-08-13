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

#define INIT_SPACE 8

typedef struct Detour Detour;

struct CityMap {
	size_t length, lengthMax;
	City **cities;
};

static bool adjust(CityMap *cityMap);
static bool empty(const CityMap *cityMap);
static void destroyLast(CityMap *cityMap);

CityMap *cityMapInit() {
	CityMap *ans = malloc(sizeof(CityMap));
	size_t lengthMax = INIT_SPACE;
	if (ans) {
		*ans = (CityMap) {
			.length = 0,
			.lengthMax = lengthMax,
			.cities = calloc(lengthMax, sizeof(City *))
		};
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
	*pCityMap = NULL;
	for (size_t i = temp->length; i > 0; --i) {
		assert(temp->length == i);
		destroyLast(temp);
	}
	free(temp->cities);
	free(temp);
}

City *cityMapAddCity(CityInfo info, City *(*fun)(CityInfo, size_t)) {
	bool success = adjust(info.cityMap);
	if (success) {
		size_t position = info.cityMap->length;
		City **pCity = &info.cityMap->cities[position];
		*pCity = fun(info, position);
		if (*pCity) {
			++info.cityMap->length;
			return *pCity;
		}
	}
	return NULL;
}

bool cityMapIsLast(const CityMap *cityMap, City *const *city) {
	City **last = &cityMap->cities[cityMap->length - 1];
	return *city == *last;
}

City *const *cityMapSuffix(CityMap *cityMap, size_t start) {
	return &cityMap->cities[start];
}

void cityMapTrim(CityMap *cityMap, size_t length) {
	if (cityMap->length < length)
		assert(false);
	while (length < cityMap->length) {
		destroyLast(cityMap);
	}
}

City *cityMapGetAt(CityMap *cityMap, size_t index) {
	return cityMap->cities[index];
}

static void destroyLast(CityMap *cityMap) {
	(void) empty; // used only by assertions
	assert(!empty(cityMap));
	City **last = &cityMap->cities[cityMap->length - 1];
	for (size_t i = cityGetRoadCount(*last); i > 0; --i) {
		assert(i == cityGetRoadCount(*last));
		cityDetachLast(*last);
	}
	cityDestroy(last);
	--cityMap->length;
}

static bool empty(const CityMap *cityMap) {
	return cityMap->length == 0;
}

static bool adjust(CityMap *cityMap) {
	if (cityMap->length < cityMap->lengthMax)
		return true;
	size_t newMax = 2 * cityMap->lengthMax;
	City **temp = realloc(cityMap->cities, newMax * sizeof(City *));
	if (temp) {
		cityMap->lengthMax = newMax;
		cityMap->cities = temp;
		return true;
	}
	return false;
}