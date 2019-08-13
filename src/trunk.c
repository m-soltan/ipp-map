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

#define ROUTE_NUMBER_MAX_LENGTH 3

struct Trunk {
	unsigned id;
	unsigned pad;
	size_t length;
	City *first,*last;
	Road **roads;
};

static bool reserve(Trunk *trunk);
static size_t descriptionLength(const Trunk *trunk);
static size_t sumOfRoads(const Trunk *trunk);
static void merge(Trunk *result, Trunk *base, Trunk *infix);
static void append(Trunk *prefix, Trunk *suffix, Trunk *result);
static City *getCity(Trunk *trunk, size_t position);
static Trunk *makeDetour(CityMap *cityMap, Trunk *trunk, Road *road);
static Trunk *rebuild(Trunk *ans, Trunk *base, size_t length);
static Trunk *join(Trunk *trunk, Trunk *extension);
static Trunk *makeExtension(CityMap *cityMap, Trunk *trunk, City *city);

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

char *trunkDescription(const Trunk *const trunk) {
	char *ans, *tmp;
	size_t size = descriptionLength(trunk);
	tmp = ans = malloc(size);
	if (ans) {
		tmp += sprintf(tmp, "%u;", trunk->id);
		assert(tmp <= 1 + ROUTE_NUMBER_MAX_LENGTH + ans);
		tmp += cityCopyName(tmp, trunk->first) - 1;
		City *current = trunk->first;
		for (size_t i = 0; i < trunk->length; ++i) {
			int written;
			City *city1, *city2;
			Road *road = trunk->roads[i];
			assert(roadHasCity(road, current));
			roadGetCities(road, &city1, &city2);
			bool isFirst = (city1 == current), isSecond = (city2 == current);
			(void) isSecond; // used only by assertions
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

Trunk *trunkBuild(City *from, City *to, CityMap *m, unsigned trunkId) {
	Trunk *ans = malloc(sizeof(Trunk));
	if (ans) {
		*ans = (Trunk) {
			.first = from,
			.last = to,
			.id = trunkId,
		};
		ans->roads = cityPath(from, to, m, &ans->length);
		if (ans->length == SIZE_MAX) { // no route
			assert(ans->roads == NULL);
			return ans;
		}
		if (ans->roads) {
			if (reserve(ans))
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
			if (ans->roads) {
				if (detour->length != SIZE_MAX) {
					merge(ans, trunk, detour);
					trunkFree(&detour);

					assert(roadHasCity(ans->roads[0], ans->first));
					assert(roadHasCity(ans->roads[ans->length - 1], ans->last));
					return ans;
				}
				free(ans->roads);
			}
			trunkFree(&detour);
		}
		free(ans);
	}
	return NULL;
}

Trunk *trunkExtend(CityMap *cityMap, Trunk *trunk, City *city) {
	Trunk *extension;
	extension = makeExtension(cityMap, trunk, city);
	if (extension != NULL)
		return join(trunk, extension);
	else
		return NULL;
}

void trunkFree(Trunk **pTrunk) {
	Trunk *trunk = *pTrunk;
	assert(trunk != NULL);
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

size_t trunkGetLength(const Trunk *trunk) {
	return trunk->length;
}

Trunk *trunkMake(unsigned id, City *first, City *last, Road *const *roads, size_t roadCount) {
	for (size_t i = 0; i < roadCount; ++i) {
		bool reserveSuccess = roadReserve(roads[i], 1);
		if (!reserveSuccess)
			return NULL;
	}
	Trunk *ans = malloc(sizeof(Trunk));
	if (ans) {
		*ans = (Trunk) {
			.id = id,
			.first = first,
			.last = last,
			.length = roadCount,
			.roads = calloc(roadCount, sizeof(Road *)),
		};
		if (ans->roads) {
			for (size_t i = 0; i < roadCount; ++i)
				ans->roads[i] = roads[i];
			return ans;
		}
		free(ans);
	}
	return NULL;
}

static size_t descriptionLength(const Trunk *trunk) {
	size_t ans = ROUTE_NUMBER_MAX_LENGTH + strlen(";");
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
			trunkFree(&extension);
			return ans;
		}
		free(ans);
		trunkFree(&extension);
	}
	return NULL;
}

static size_t getPosition(Trunk *trunk, Road *road) {
	for (size_t i = 0; i < trunk->length; ++i)
		if (trunk->roads[i] == road)
			return i;
	assert(false);
	return trunk->length;
}

static Trunk *makeDetour(CityMap *cityMap, Trunk *trunk, Road *road) {
	size_t position = getPosition(trunk, road);
	unsigned length;
	City *from, *to;
	from = getCity(trunk, position);
	to = getCity(trunk, 1 + position);
	Trunk *ans;
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
	for (detourStart = 0; true; ++detourStart)
		if (infix->first == getCity(base, detourStart))
			break;
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
	result->first = prefix->first;
	result->last = suffix->last;
}

static Trunk *makeExtension(CityMap *cityMap, Trunk *trunk, City *city) {
	Trunk *ans, *trunk1, *trunk2;
	cityUnblock(trunk->first);
	ans = trunk1 = trunkBuild(city, trunk->first, cityMap, trunk->id);
	cityBlock(trunk->first);
	if (trunk1 == NULL)
		return NULL;
	cityUnblock(trunk->last);
	trunk2 = trunkBuild(trunk->last, city, cityMap, trunk->id);
	cityBlock(trunk->last);
	if (trunk2 == NULL) {
		trunkFree(&trunk1);
		return NULL;
	}
	if (sumOfRoads(trunk1) < sumOfRoads(trunk2))
		trunk1 = trunk2;
	else
		ans = trunk2;
	assert(ans != trunk1);
	trunkFree(&trunk1);
	if (ans->length != SIZE_MAX) {
		trunkAttach(ans);
		return ans;
	}
	trunkFree(&ans);
	return NULL;
}

static bool reserve(Trunk *trunk) {
	for (size_t i = 0; i < trunk->length; ++i)
		if (!roadReserve(trunk->roads[i], ROUTE_LIMIT))
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

static size_t sumOfRoads(const Trunk *trunk) {
	size_t ans = 0;
	if (trunk->length == SIZE_MAX) {
		assert(trunk->roads == NULL);
		return SIZE_MAX;
	}
	for (size_t i = 0; i < trunk->length; ++i) {
		size_t length = roadGetLength(trunk->roads[i]);
		assert(SIZE_MAX - length > ans);
		ans += length;
	}
	return ans;
}

static City *getCity(Trunk *trunk, size_t position) {
	Road *road1, *road2;
	if (position == 0) {
		return trunk->first;
	}
	if (position == trunk->length) {
		return trunk->last;
	}
	road1 = trunk->roads[position - 1];
	road2 = trunk->roads[position];
	return roadIntersect(road1, road2);

}
