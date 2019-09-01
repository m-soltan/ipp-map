#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "city.h"
#include "city_map.h"
#include "road.h"
#include "trie.h"
#include "trunk.h"

#define ROUTE_NUMBER_MAX_LENGTH 3

struct Trunk {
	unsigned id;
	unsigned pad;
	size_t length;
	City *first,*last;
	Road **roads;
};

static bool isDecoy(const Trunk *trunk);
static bool reserve(Road *const *roads, size_t count);
static int getMinYear(const Trunk *trunk);
static size_t descriptionLength(const Trunk *trunk);
static size_t sumOfRoads(const Trunk *trunk);
static void append(Trunk *prefix, Trunk *suffix, Trunk *result);
static void block(Trunk *trunk);
static void merge(Trunk *result, Trunk *base, Trunk *infix);
static void unblock(Trunk *trunk);
static City *getCity(Trunk *trunk, size_t position);
static Trunk decoy(void);
static Trunk *makeDetour(CityMap *cityMap, Trunk *trunk, Road *road);
static Trunk *rebuild(Trunk *ans, Trunk *base, size_t length);
static Trunk *join(Trunk *trunk, Trunk *extension);
static Trunk *chooseExtension(Trunk **pTrunk1, Trunk **pTrunk2);
static Trunk *makeExtension(CityMap *cityMap, Trunk *trunk, City *city);

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
			long written;
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

void trunkAttach(Trunk *trunk) {
	for (size_t i = 0; i < trunk->length; ++i)
		roadTrunkAdd(trunk->roads[i], trunk->id);
}

Trunk *trunkBuild(City *from, City *to, CityMap *m, unsigned trunkId) {
	Trunk *ans = calloc(1, sizeof(Trunk));
	if (ans) {
		*ans = (Trunk) {
			.first = from,
			.last = to,
			.id = trunkId,
		};
		ans->roads = cityPath(from, to, m, &ans->length);
		if (isDecoy(ans)) // no route
			return ans;
		if (ans->roads) {
			if (reserve(ans->roads, ans->length))
				return ans;
			free(ans->roads);
		}
		free(ans);
	}
	return NULL;
}

Trunk *trunkAddDetour(CityMap *cityMap, Trunk *trunk, Road *road) {
	Trunk *ans = calloc(1, sizeof(Trunk));
	assert(trunk);
	if (ans) {
		Trunk *detour = makeDetour(cityMap, trunk, road);
		if (detour) {
			if (!isDecoy(detour)) {
				rebuild(ans, trunk, detour->length + trunk->length - 1);
				if (ans->roads) {
					merge(ans, trunk, detour);
					trunkFree(&detour);
					assert(ans->id < ROUTE_LIMIT);
					assert(roadHasCity(ans->roads[0], ans->first));
					assert(roadHasCity(ans->roads[ans->length - 1], ans->last));
					return ans;
				}
			}
			trunkFree(&detour);
		}
		free(ans);
	}
	return NULL;
}

Trunk *trunkExtend(CityMap *cityMap, Trunk *trunk, City *city) {
	Trunk *extension;
	block(trunk);
	extension = makeExtension(cityMap, trunk, city);
	unblock(trunk);
	if (extension) {
		if (!isDecoy(extension))
			return join(trunk, extension);
		trunkFree(&extension);
	}
	return NULL;
}

void trunkFree(Trunk **pTrunk) {
	assert(pTrunk && *pTrunk);
	Trunk *trunk = *pTrunk;
	*pTrunk = NULL;
	if (!isDecoy(trunk))
		free(trunk->roads);
	free(trunk);
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

bool trunkTest(const Trunk *trunk) {
	Road **const roads = trunk->roads;
	for (size_t i = 1; i < trunk->length; ++i) {
		if (!roadHasRoute(roads[i], trunk->id)) {
			bp();
			return false;
		}
		if (roads[i - 1] == roads[i]) {
			bp();
			return false;
		}
		if (!roadHasIntersection(roads[i - 1], roads[i])) {
			bp();
			return false;
		}
	}
	return true;
}

Trunk *trunkMake(Trie *trie, unsigned id, NameList list) {
	const size_t roadCount = list.length - 1;
	Trunk *ans = calloc(1, sizeof(Trunk));
	if (ans) {
		City *first = trieFind(trie, list.v[0]);
		City *last = trieFind(trie, list.v[list.length - 1]);
		assert(first && last);
		*ans = (Trunk) {
			.length = roadCount,
			.roads = calloc(roadCount, sizeof(Road *)),
			.id = id,
			.first = first,
			.last = last,
		};
		for (size_t i = 0; i < roadCount; ++i) {
			City *city1 = trieFind(trie, list.v[i]);
			City *city2 = trieFind(trie, list.v[i + 1]);
			Road *road = cityFindRoad(city1, city2);
			assert(road);
			ans->roads[i] = road;
		}
		return ans;
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
	assert(trunkTest(trunk));
	from = getCity(trunk, position);
	to = getCity(trunk, 1 + position);
	Trunk *ans;
	block(trunk);
	length = roadBlock(road);
	ans = trunkBuild(from, to, cityMap, trunk->id);
	roadUnblock(road, length);
	unblock(trunk);
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
	Trunk *trunk1, *trunk2;
	cityUnblock(trunk->first);
	trunk1 = trunkBuild(city, trunk->first, cityMap, trunk->id);
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
	return chooseExtension(&trunk1, &trunk2);
}

static bool reserve(Road *const *roads, const size_t count) {
	for (size_t i = 0; i < count; ++i) {
		bool reserveSuccess = roadReserve(roads[i]);
		if (reserveSuccess == false)
			return false;
	}
	return true;
}

static Trunk *rebuild(Trunk *const ans, Trunk *base, size_t length) {
	assert(length != SIZE_MAX);
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

Trunk *chooseExtension(Trunk **pTrunk1, Trunk **pTrunk2) {
	enum {first, second, none} choice;
	assert(pTrunk1 && pTrunk2);
	Trunk *trunk1 = *pTrunk1, *trunk2 = *pTrunk2;
	*pTrunk1 = *pTrunk2 = NULL;
	if (isDecoy(trunk1) && isDecoy(trunk2)) {
		choice = none;
	} else if (isDecoy(trunk1)) {
		choice = second;
	} else if (isDecoy(trunk2)) {
		choice = first;
	} else {
		const size_t sum1 = sumOfRoads(trunk1), sum2 = sumOfRoads(trunk2);
		if (sum1 < sum2) {
			choice = first;
		} else if (sum1 > sum2) {
			choice = second;
		} else {
			const int year1 = getMinYear(trunk1), year2 = getMinYear(trunk2);
			if (year1 < year2) {
				choice = first;
			} else if (year1 > year2) {
				choice = second;
			} else {
				choice = none;
			}
		}
	}

	switch (choice) {
		case (first) : {
			trunkFree(&trunk2);
			assert(trunk1);
			return trunk1;
		}
		case (second) : {
			trunkFree(&trunk1);
			assert(trunk2);
			return trunk2;
		}
		case (none) : {
			trunkFree(&trunk2);
			free(trunk1->roads);
			*trunk1 = decoy();
			return trunk1;
		}
		default: {
			assert(false);
			return NULL;
		}
	}
}
int getMinYear(const Trunk *trunk) {
	int32_t ans = INT32_MAX;
	for (size_t i = 0; i < trunk->length; ++i) {
		int year = roadGetYear(trunk->roads[i]);
		if (year < ans)
			ans = year;
	}
	assert(ans != INT32_MAX);
	return ans;
}

static Trunk decoy() {
	return (Trunk) {.length = SIZE_MAX};
}

static bool isDecoy(const Trunk *trunk) {
	if (trunk->length == SIZE_MAX){
		assert(trunk->roads == NULL);
		return true;
	} else {
		return false;
	}
}


static void unblock(Trunk *trunk) {
	for (size_t i = 0; i < trunk->length; i += 2) {
		City *city1, *city2;
		roadGetCities(trunk->roads[i], &city1, &city2);
		cityUnblock(city1);
		cityUnblock(city2);
	}
	if (trunk->length % 2 == 0)
		cityUnblock(trunk->last);
}

static void block(Trunk *trunk) {
	for (size_t i = 0; i < trunk->length; i += 2) {
		City *city1, *city2;
		roadGetCities(trunk->roads[i], &city1, &city2);
		cityBlock(city1);
		cityBlock(city2);
	}
	if (trunk->length % 2 == 0)
		cityBlock(trunk->last);
}
