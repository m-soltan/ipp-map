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

static bool reserve(Trunk trunk);
static size_t descriptionLength(const Trunk *trunk);
static Trunk *makeDetour(CityMap *cityMap, Trunk *trunk, Road *road);
static Trunk *rebuild(Trunk *ans, Trunk *base, size_t length);
static Trunk *join(Trunk *trunk, Trunk *extension);
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

const char *trunkDescription(const Trunk *const trunk) {
	char *ans, *tmp;
	size_t size = descriptionLength(trunk);
	tmp = ans = malloc(size);
	if (ans) {
		tmp += sprintf(tmp, "%u;", trunk->id);
		assert(tmp <= strlen("999;") + ans);
		tmp += cityGetName(tmp, trunk->first) - 1;
		City *current = trunk->first;
		for (size_t i = 0; i < trunk->length; ++i) {
			int written;
			City *city1, *city2;
			Road *road = trunk->roads[i];
			assert(roadHasCity(road, current));
			roadGetCities(road, &city1, &city2);
			bool isFirst = (city1 == current), isSecond = (city2 == current);
			assert(isFirst || isSecond);
			current = (isFirst ? city2 : city1);
			written = roadWrite(tmp, road, current) - 1;
			assert(written > 0);
			tmp += written;
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
	for (size_t i = 0; i < trunk->length; ++i)
		roadTrunkAdd(trunk->roads[i], trunk->id);
}

Trunk *trunkBuild(City *from, City *to, CityMap *m, unsigned number) {
	Trunk *ans = malloc(sizeof(Trunk));
	if (ans) {
		*ans = (Trunk) {
			.first = from,
			.last = to,
			.id = number,
		};
		ans->roads = cityPath(from, to, m, &ans->length);
		if (ans->length == SIZE_MAX) { // no route
			assert(ans->roads == NULL);
			return ans;
		}
		if (ans->roads) {
			if (reserve(*ans))
				return ans;
			free(ans->roads);
		}
		free(ans);
	}
	return NULL;
}

Trunk *trunkAddDetour(CityMap *cityMap, Trunk *trunk, Road *road) {
	Trunk *ans = malloc(sizeof(Trunk));
	if (ans) {
		Trunk *detour = makeDetour(cityMap, trunk, road);
		if (detour) {
			rebuild(ans, trunk, detour->length + trunk->length - 1);
			if (ans->roads && detour->length != SIZE_MAX) {
				merge(ans, trunk, detour);
				free(detour->roads);
				free(detour);
				assert(roadHasCity(ans->roads[0], ans->first));
				assert(roadHasCity(ans->roads[ans->length - 1], ans->last));
				return ans;
			}
			free(detour->roads);
			free(detour);
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
	assert(*pTrunk != NULL);
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

unsigned trunkGetId(Trunk *trunk) {
	return trunk->id;
}

void trunkDestroy(Trunk **pTrunk) {
	Trunk *trunk = *pTrunk;
	for (size_t i = 0; i < trunk->length; ++i)
		roadTrunkRemove(trunk->roads[i], trunk->id);
	free(trunk->roads);
	free(*pTrunk);
	*pTrunk = NULL;
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

static Trunk *makeDetour(CityMap *cityMap, Trunk *trunk, Road *road) {
	City *from, *to;
	Trunk *ans;
	unsigned length;
	roadGetCities(road, &from, &to);
	trunkBlock(trunk);
	length = roadBlock(road);
	ans = trunkBuild(from, to, cityMap, trunk->id);
	roadUnblock(road, length);
	trunkUnblock(trunk);
	return ans;
}

static void merge(Trunk *result, Trunk *base, Trunk *infix) {
	size_t detourStart;
	assert(infix->first != NULL && infix->last != NULL);
	for (detourStart = 1; true; ++detourStart) {
		if (roadHasCity(base->roads[detourStart - 1], infix->first))
			break;
		assert(infix->first != NULL);
	}
	for (size_t i = 0; i < result->length; ++i) {
		if (i < detourStart)
			result->roads[i] = base->roads[i];
		else if (i < detourStart + infix->length)
			result->roads[i] = infix->roads[i - detourStart];
		else
			result->roads[i] = base->roads[i - infix->length + 1];
	}
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

static bool reserve(Trunk trunk) {
	for (size_t i = 0; i < trunk.length; ++i)
		if (!roadReserve(trunk.roads[i], ROUTE_LIMIT))
			return false;
	return true;
}

static Trunk *rebuild(Trunk *const ans, Trunk *base, size_t length) {
	*ans = (Trunk) {
		.first = base->first,
		.last = base->last,
		.id = base->id,
		.length = length,
		.roads = malloc(length * sizeof(City *))
	};
	return ans;
}

