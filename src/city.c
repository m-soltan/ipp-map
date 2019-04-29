#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "city.h"
#include "queue.h"
#include "trie.h"
#include "trunk.h"

#define BUFFER_START_LENGTH 256
#define INIT_ROUTE_MAX 8

typedef struct Record Record;

struct City {
	bool blocked;
	size_t id, nameLength, roadCount, roadMax;
	char *name;
	Road **roads;
};

struct CityMap {
	size_t length, lengthMax;
	City **cities;
};

struct Record {
	bool *seen;
	City **prev;
};

struct Road {
	City *city1, *city2;
	size_t *routes;
	unsigned length;
	int year;
	size_t routeCount, routeMax;
};

// auxiliary function declarations
bool cityMapAdjust(CityMap *cityMap);
bool roadInsert(CityMap *cityMap, Trie *t, Road **road, Trunk *trunks[1000]);
City **makeList(City *from, City *to, Record *record, size_t *length);
City **reverse(City **list, size_t length);
void recordFree(Record **pRecord);
Record *recordMake(size_t size);
void initFields(City *city);
void visit(Heap *queue, City *current, size_t distance, Record *record, int minYear);
void cityDestroy(City **city, CityMap *cityMap);

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

void cityMapDestroy(CityMap **pCityMap) {
	CityMap *temp = *pCityMap;
	while (temp->length > 0) {
		City **c = &temp->cities[temp->length - 1];
		while ((*c)->roadCount > 0) {
			Road *r = (*c)->roads[(*c)->roadCount - 1];
			cityDetach(*c, r);
			if (r->city1 == *c)
				r->city1 = NULL;
			else
				r->city2 = NULL;
			if (r->city1 == NULL && r->city2 == NULL) {
				free(r->routes);
				free(r);
			}
		}
		cityDestroy(c, temp);
	}
}

bool cityMakeSpace(City *city) {
	assert(city->roadCount <= city->roadMax);
	if (city->roadCount < city->roadMax)
		return true;
	size_t tmpMax = 2 * city->roadMax;
	Road **tmp = realloc(city->roads, tmpMax * sizeof(Road *));
	if (tmp == NULL)
		return false;
	city->roads = tmp;
	city->roadMax *= 2;
	return true;
}

bool cityAddRoad(City *city, Road *road) {
	if (!cityMakeSpace(city))
		return false;
	city->roads[city->roadCount] = road;
	++city->roadCount;
	return true;
}

size_t cityGetNameLength(const City *city) {
	return city->nameLength - 1;
}

void cityGetName(char *dest, const City *city) {
	strncpy(dest, city->name, city->nameLength);
}

bool roadAdjust(const City *from, const City *to) {
	Road *road = roadFind(from, to);
	assert(road);
	if (road->routeCount < road->routeMax)
		return true;
	if (road->routes == NULL) {
		road->routes = malloc(INIT_ROUTE_MAX * sizeof(Trunk *));
		if (road->routes) {
			road->routeMax = INIT_ROUTE_MAX;
			return true;
		} else {
			return false;
		}
	}
	assert(road->routeMax != 0);
	size_t *temp = realloc(road->routes, 2 * road->routeMax * sizeof(size_t));
	if (!temp)
		return false;
	road->routes = temp;
	road->routeMax *= 2;
	return true;
}

bool roadDestroy(CityMap *cityMap, Road *road, Trunk *trunks[1000]) {
	if (road->routes != NULL) {
		Trunk **replacements = malloc(road->routeCount * sizeof(Trunk *));
		size_t length = road->length;
		if (!replacements)
			return false;
		road->length = 0;
		for (size_t i = 0; i < road->routeCount; ++i) {
			Trunk *t = trunks[road->routes[i]];
			replacements[i] = trunkDetour(cityMap, t);
			if (replacements[i] == NULL) {
				road->length = length;
				for (size_t j = 0; j < i; ++j)
					trunkFree(&replacements[i]);
				free(replacements);
				return false;
			}
		}
	}
	cityDetach(road->city1, road);
	cityDetach(road->city2, road);
	return true;
}

bool roadExtend(CityMap *m, Trie *t, City *city, RoadInfo info) {
	bool successAdd, successInsert;
	const char *str = (info.city1 ? info.city1 : info.city2);
	Road *r = malloc(sizeof(Road));
	if (r) {
		City *c = cityInit(m, str, r);
		if (c) {
			successAdd = cityAddRoad(city, r);
			if (successAdd) {
				trieInsert(t, str, &c, &successInsert);
				if (successInsert) {
					*r = (Road) {.length = info.length, .year = info.builtYear};
					r->city1 = (c < city ? c : city);
					r->city2 = (c < city ? city : c);
					r->routes = NULL;
					return true;
				}
				cityDetach(city, r);
			}
			cityDestroy(&c, m);
		}
		free(r);
	}
	return false;
}

bool roadLink(City *city1, City *city2, unsigned length, int year) {
	if (roadFind(city1, city2) != NULL)
		return false;
	Road *r = malloc(sizeof(Road));
	if (r) {
		if (cityAddRoad(city1, r)) {
			if (cityAddRoad(city2, r)) {
				*r = (Road) {.length = length, .year = year};
				r->city1 = (city1 < city2 ? city1 : city2);
				r->city2 = (city1 < city2 ? city2 : city1);
				return true;
			}
			--city1->roadCount;
		}
		free(r);
	}
	return false;
}

City *cityInit(CityMap *map, const char *name, Road *road) {
	City **ans, *c;
	assert(name != NULL);
	cityMapAdjust(map);
	ans = &map->cities[map->length];
	*ans = malloc(sizeof(City));
	**ans = (City) {.id = map->length, .nameLength = 1 + strlen(name)};
	c = *ans;
	initFields(c);
	c->roads = malloc(c->roadMax * sizeof(Road));
	if (c->roads) {
		c->roads[0] = road;
		c->name = malloc(c->nameLength);
		if (c->name) {
			strncpy(c->name, name, c->nameLength);
			c->name[c->nameLength - 1] = '\0';
			++map->length;
			return c;
		}
		free(c->roads);
	}
	return NULL;
}

bool roadInit(CityMap *m, Trie *t, RoadInfo info, Trunk *trunks[1000]) {
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
		ans = NULL;
		if (c1)
			free(c1);
		if (c2)
			free(c2);
		return NULL;
	}
	return roadInsert(m, t, &ans, trunks);
}

bool roadUpdate(Road *road, int year) {
	if (year != 0 && year >= road->year) {
		road->year = year;
		return true;
	}
	return false;
}

int roadGetYear(const Road *road) {
	return road->year;
}

Road * roadFind(const City *city1, const City *city2) {
	if (city1 == city2)
		return NULL;
	if (city1 > city2)
		return roadFind(city2, city1);
	size_t n = city1->roadCount;
	Road **p = city1->roads;
	for (size_t i = 0; i < n; ++i) {
		if (p[i]->city2 == city2)
			return p[i];
	}
	return NULL;
}

size_t roadWrite(char *str, const City *city1, const City *city2) {
	int ans;
	Road *road = roadFind(city1, city2);
	ans = sprintf(str, ";%u;%i;%s", road->length, road->year, city2->name);
	assert(ans > 0);
	return (size_t) ans;
}

unsigned roadBlock(City *from, City *to) {
	Road *road = roadFind(from, to);
	unsigned ans = road->length;
	from->blocked = to->blocked = true;
	road->length = 0;
	return ans;
}

void cityDetach(City *city, const Road *road) {
	Road **last = &city->roads[city->roadCount - 1], *temp = *last;
	for (unsigned i = 0; i < city->roadCount; ++i) {
		if (city->roads[i] == road) {
			*last = city->roads[i];
			city->roads[i] = temp;
			--city->roadCount;
		}
	}
}

void roadTrunkAdd(Road *r, size_t trunk) {
	assert(r->routeCount < r->routeMax);
	r->routes[r->routeCount] = trunk;
	++r->routeCount;
}

void roadUnblock(Road *road, unsigned length) {
	assert(road->length == 0);
	road->city1->blocked = road->city2->blocked = false;
	road->length = length;
}

// auxiliary function definitions
bool cityMapAdjust(CityMap *cityMap) {
	City **temp;
	size_t size;
	if (cityMap->length == cityMap->lengthMax) {
		assert(cityMap->lengthMax);
		size = 2 * cityMap->lengthMax;
		temp = realloc(cityMap->cities, size * sizeof(City *));
		if (temp != NULL) {
			cityMap->cities = temp;
			cityMap->lengthMax *= 2;
			return true;
		} else {
			return false;
		}
	}
	return true;
}

bool roadInsert(CityMap *cityMap, Trie *t, Road **road, Trunk *trunks[1000]) {
	bool success1, success2;
	Trie **parent1;
	City 	**city1 = &(*road)->city1, **city2 = &(*road)->city2;
	parent1 = trieInsert(t, (*city1)->name, city1, &success1);
	if (success1) {
		trieInsert(t, (*city2)->name, city2, &success2);
		if (success2)
			return true;
		if (parent1)
			trieDestroy(parent1);
	}
	roadDestroy(cityMap, *road, trunks);
	return false;
}

City **makeList(City *from, City *to, Record *record, size_t *length) {
	size_t bufferSize = BUFFER_START_LENGTH * sizeof(City *);
	size_t ansLength = 1;
	for (City *c = to; c != from; c = record->prev[c->id])
		++ansLength;
	City *current = to, **buffer = malloc(ansLength * sizeof(City *));
	for (*length = 0; *length < ansLength; current = record->prev[current->id]) {
		if (*length > bufferSize) {
			City **temp;
			bufferSize *= 2;
			temp = realloc(buffer, bufferSize);
			if (!temp) {
				free(buffer);
				return NULL;
			}
			buffer = temp;
		}
		buffer[*length] = current;
		++*length;
		assert(current);
	}
	return buffer;
}

City **reverse(City **list, size_t length) {
	City *temp;
	for (size_t i = 0; i < length - i - 1; ++i) {
		temp = list[i];
		list[i] = list[length - i - 1];
		list[length - i - 1] = temp;
	}
	return list;
}

City **cityPath(City *from, City *to, CityMap *map, size_t *length) {
	City **ans, *check = NULL, *current, *prev, *ptrBack;
	size_t d1, d2 = SIZE_MAX;
	Heap *queue;
	int minYear = INT16_MAX, minYear2 = INT16_MAX;
	Record *record = recordMake(map->length);
	current = from;
	if (!record)
		return NULL;
	record->prev[from->id] = from;
	queue = queueInit();
	if (!queue) {
		recordFree(&record);
		return NULL;
	}
	for (d1 = 0; current != to;) {
		assert(current != NULL);
		visit(queue, current, d1, record, minYear);
		if (queueEmpty(queue)) {
			queueDestroy(&queue);
			recordFree(&record);
			return NULL;
		}
		current = queuePop(queue, &d1, &minYear, &ptrBack);
		record->prev[current->id] = ptrBack;
	}
	if (!queueEmpty(queue))
		check = queuePop(queue, &d2, &minYear2, &prev);
	if (check == to && d2 == d1 && minYear2 == minYear)
		ans = NULL;
	else
		ans = reverse(makeList(from, to, record, length), *length);
	queueDestroy(&queue);
	recordFree(&record);
	return ans;
}

void recordFree(Record **pRecord) {
	Record *record = *pRecord;
	free(record->prev);
	free(record->seen);
	free(record);
	*pRecord = NULL;
}

Record *recordMake(size_t size) {
	Record *ans = malloc(sizeof(Record));
	if (ans) {
		ans->seen = calloc(size, sizeof(bool));
		if (ans->seen) {
			ans->prev = malloc(size * sizeof(City *));
			if (ans->prev)
				return ans;
		}
		free(ans);
	}
	return NULL;
}

void initFields(City *city) {
	city->blocked = false;
	city->roadCount = 1;
	city->roadMax = 8;
}

void visit(Heap *queue, City *current, size_t distance, Record *record, int minYear) {
	for (size_t i = 0; i < current->roadCount; ++i) {
		Road *r = current->roads[i];
		if (r->length == 0)
			continue;
		minYear = (roadGetYear(r) < minYear ? roadGetYear(r) : minYear);
		City *nextCity = (r->city1 != current ? r->city1 : r->city2);
		record->seen[current->id] = true;
		if (record->seen[nextCity->id] || nextCity->blocked)
			continue;
		if (r->city1 != current)
			queuePush(queue, r->city1, current, distance + r->length, minYear);
		else
			queuePush(queue, r->city2, current, distance + r->length, minYear);
	}
}

void cityDestroy(City **city, CityMap *cityMap) {
	City **last = cityMap->cities + cityMap->length - 1, *temp = *last;
	if (*city != *last) {
		temp->id = (*city)->id;
		*last = *city;
		*city = temp;
	}
	free(temp->name);
	free(temp->roads);
	--cityMap->length;
	*city = NULL;
}
