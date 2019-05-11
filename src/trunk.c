#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "city.h"
#include "city_map.h"
#include "road.h"
#include "trunk.h"

struct Trunk {
	City *first,*last;
	unsigned id;
	char pad[sizeof(size_t) - sizeof(unsigned)];
	size_t length;
	Road **roads;
};

struct TrunkOld {
	size_t length, id;
	City **cities;
};

static size_t descriptionLength(const Trunk *trunk);
static Trunk *join(Trunk *trunk, Trunk *extension);
static Trunk makeDetour(CityMap *cityMap, Trunk *trunk, Road *road);
static Trunk *makeExtension(CityMap *cityMap, Trunk *trunk, City *city);
static void merge(Trunk *result, Trunk *base, Trunk *infix);
static void append(Trunk *prefix, Trunk *suffix, Trunk *result);

// linked function definitions
bool trunkHasCity(const Trunk *trunk, const City *city) {
	for (size_t i = 0; i < trunk->length; ++i) {
		City *city1, *city2;
		roadGetCities(trunk->roads[i], &city1, &city2);
		if (city1 == city || city == city2)
			return true;
	}
	return false;
}

const char *trunkDescription(const Trunk *trunk) {
	char *ans, *tmp;
	size_t size = descriptionLength(trunk);
	tmp = ans = malloc(size);
	if (ans) {
		City *current = trunk->first;
		tmp += sprintf(tmp, "%u;", trunk->id);
		assert(tmp <= strlen("999;") + ans);
		tmp += cityGetName(tmp, trunk->first);
		for (size_t i = 0; i < trunk->length; ++i) {
			Road *road = trunk->roads[i];
			current = roadGetOther(road, current);
			tmp += roadWrite(tmp, road, current);
		}
	}
	return ans;
}

void trunkBlock(Trunk *trunk) {
	for (size_t i = 0; i < trunk->length; i += 2) {
		City *city1, *city2;
		roadGetCities(trunk->roads[i], &city1, &city2);
		cityBlock(city1);
		cityBlock(city2);
	}
	if (trunk->length % 2 == 0)
		cityBlock(trunk->last);
}

void trunkAttach(Trunk *trunk) {
	for (size_t i = 1; i < trunk->length; ++i)
		roadTrunkAdd(trunk->roads[i], trunk->id);
}

// TODO: distinguish no route and no memory
Trunk *trunkBuild(City *from, City *to, CityMap *m, unsigned number) {
	Trunk *ans = malloc(sizeof(Trunk));
	if (ans) {
		ans->roads = cityPath(from, to, m, &ans->length);
		ans->id = number;
		if (ans->roads)
			return ans;
		free(ans);
	}
	return NULL;
}

Trunk *trunkDetour(CityMap *cityMap, Trunk *trunk, Road *road) {
	Trunk *ans = malloc(sizeof(TrunkOld));
	if (ans) {
		Trunk detour = makeDetour(cityMap, trunk, road);
		if (detour.roads) {
			ans->id = trunk->id;
			ans->length = detour.length + trunk->length;
			ans->id = trunk->id;
			ans->roads = malloc(ans->length * sizeof(City *));
			if (ans->roads) {
				merge(ans, trunk, &detour);
				free(detour.roads);
				return ans;
			}
			free(detour.roads);
		}
		free(ans);
	}
	return NULL;
}

Trunk *trunkExtend(CityMap *cityMap, Trunk *trunk, City *city) {
	Trunk *extension;
	extension = makeExtension(cityMap, trunk, city);
	return join(trunk, extension);
}

void trunkFree(Trunk **pTrunk) {
	Trunk *trunk = *pTrunk;
	*pTrunk = NULL;
	free(trunk->roads);
	free(trunk);
}

void trunkUnblock(Trunk *trunk) {
	for (size_t i = 0; i < trunk->length; i += 2) {
		City *city1, *city2;
		roadGetCities(trunk->roads[i], &city1, &city2);
		cityUnblock(city1);
		cityUnblock(city2);
	}
	if (trunk->length % 2 == 0)
		cityUnblock(trunk->last);
}

static size_t descriptionLength(const Trunk *trunk) {
	size_t ans = strlen("999;");
	const size_t lengthLen = 3 * sizeof(unsigned) + strlen(";");
	const size_t yearLen = 3 * sizeof(int) + strlen(";");
	ans += trunk->length * (lengthLen + yearLen + strlen(";"));
	for (size_t i = 0; i < trunk->length; i += 2) {
		City *city1, *city2;
		roadGetCities(trunk->roads[i], &city1, &city2);
		ans += cityGetNameLength(city1);
		ans += cityGetNameLength(city2);
	}
	if (trunk->length % 2 == 0)
		ans += cityGetNameLength(trunk->last);
	return ans;
}

static Trunk *join(Trunk *trunk, Trunk *extension) {
	Trunk *ans = malloc(sizeof(Trunk));
	if (ans) {
		ans->length = trunk->length + extension->length;
		ans->roads = malloc(ans->length * sizeof(City *));
		if (ans->roads) {
			if (trunk->last == extension->first)
				append(trunk, extension, ans);
			else
				append(extension, trunk, ans);
			ans->id = trunk->id;
			assert(ans->id == extension->id);
			free(extension);
			return ans;
		}
		free(ans);
		free(extension);
	}
	return NULL;
}

// TODO: reserve space for the detour
Trunk makeDetour(CityMap *cityMap, Trunk *trunk, Road *road) {
	City *from, *to;
	Trunk ans;
	unsigned length;
	roadGetCities(road, &from, &to);
	trunkBlock(trunk);
	length = roadBlock(road);
	ans.roads = cityPath(from, to, cityMap, &ans.length);
	roadUnblock(road, length);
	trunkUnblock(trunk);
	return ans;
}

static void merge(Trunk *result, Trunk *base, Trunk *infix) {
	size_t ansI, trunkI;
	for (ansI = trunkI = 0; true; ++ansI, ++trunkI) {
		if (base->roads[trunkI] == infix->roads[0])
			break;
		assert(trunkI < base->length);
		result->roads[ansI] = base->roads[trunkI];
	}
	assert(ansI == trunkI);
	for (; ansI - trunkI < infix->length; ++ansI)
		result->roads[ansI] = infix->roads[ansI - trunkI];
	for (; ansI < result->length; ++ansI, ++trunkI)
		result->roads[ansI] = base->roads[trunkI];
}

void append(Trunk *prefix, Trunk *suffix, Trunk *result) {
	for (size_t i = 0; i < prefix->length; ++i)
		result->roads[i] = prefix->roads[i];
	for (size_t i = 0; i < suffix->length; ++i)
		result->roads[prefix->length + i] = suffix->roads[i];
}

Trunk *makeExtension(CityMap *cityMap, Trunk *trunk, City *city) {
	Trunk *ans, *trunk1, *trunk2;
	ans = trunk1 = trunkBuild(city, trunk->first, cityMap, trunk->id);
	if (trunk1 == NULL)
		return NULL;
	trunk2 = trunkBuild(trunk->last, city, cityMap, trunk->id);
	if (trunk2 == NULL) {
		free(trunk1);
		return NULL;
	}
	if (trunk1->length < trunk2->length)
		trunk1 = trunk2;
	else
		ans = trunk2;
	assert(ans != trunk1);
	free(trunk1);
	if (ans->length != SIZE_MAX)
		return ans;
	free(ans);
	return NULL;
}
