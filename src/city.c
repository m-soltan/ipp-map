#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "city.h"
#include "queue.h"
#include "trunk.h"

#define BUFFER_START_LENGTH 256

typedef struct Record Record;

struct City {
	size_t id, nameLen, roadCount, roadMax;
	char *name;
	Road **roads;
};

struct CityMap {
	size_t size, sizeMax;
	City *cities;
};

struct Record {
	bool *seen;
	City **prev;
	size_t size;
};

struct Road {
	City *city1, *city2;
	Trunk **routes;
	unsigned length;
	int year;
};

// auxiliary function declarations
bool roadInsert(Trie *t, Road **road);
City **makeList(City *from, City *to, Record *record, size_t *length);
City **reverse(City **list, size_t length);
void recordFree(Record **pRecord);
Record *recordMake(size_t size);
//City **shortestPath(City *from, City *to, CityMap *map);
void initFields(City *city);
void takeCity(Heap *queue, City *current, size_t distance, Record *record);
void cityDestroy(City **city);

// linked function definitions
CityMap *cityMapInit() {
	CityMap *ans = malloc(sizeof(CityMap));
	if (ans) {
		*ans = (CityMap) {.size = 0, .sizeMax = 8};
		ans->cities = malloc(ans->sizeMax * sizeof(City));
		if (ans->cities)
			return ans;
		free(ans);
	}
	return NULL;
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

// TODO
bool roadDestroy(Road *road) {
	City *city1 = road->city1, *city2 = road->city2;
	cityDetach(city1, road);
	cityDetach(city2, road);
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
			cityDestroy(&c);
		}
		free(r);
	}
	return false;
}

// TODO
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
	if (map->size == map->sizeMax) {
		City *temp = realloc(map->cities, 2 * map->sizeMax);
		if (!temp)
			return NULL;
		map->cities = temp;
		map->sizeMax *= 2;
	}
	City *ans = map->cities + map->size;
	*ans = (City) {.id = map->size, .nameLen = strlen(name)};
	initFields(ans);
	ans->roads = malloc(ans->roadMax * sizeof(Road));
	if (ans->roads) {
		ans->roads[0] = road;
		ans->name = malloc(ans->nameLen);
		if (ans->name) {
			strncpy(ans->name, name, ans->nameLen);
			++map->size;
			assert(name != NULL);
			return ans;
		}
		free(ans->roads);
	}
	free(ans);
	return NULL;
}

bool roadInit(CityMap *m, Trie *t, RoadInfo info) {
	Road *ans = malloc(sizeof(Road));
	if (!ans)
		return NULL;
	City *c1 = cityInit(m, info.city1, ans), *c2 = cityInit(m, info.city2, ans);
	if (c1 && c2) {
		*ans = (Road) {.length = info.length, .year = info.builtYear};
		ans->city1 = (c1 < c2 ? c1 : c2);
		ans->city2 = (c1 < c2 ? c2 : c1);
	} else {
		free(ans);
		ans = NULL;
		if (c1)
			free(c1);
		if (c2)
			free(c2);
	}
	return roadInsert(t, &ans);
}

bool roadUpdate(Road *road, int year) {
	if (year != 0) {
		road->year = year;
		return true;
	}
	return false;
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

void cityDetach(City *city, const Road *road) {
	Road **last = &city->roads[city->roadCount - 1], *temp = *last;
	for (int i = 0; i < city->roadCount; ++i) {
		if (city->roads[i] == road) {
			*last = city->roads[i];
			city->roads[i] = temp;
			--city->roadCount;
		}
	}
}

// auxiliary function definitions
bool roadInsert(Trie *t, Road **road) {
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
	roadDestroy(*road);
	return false;
}

City **makeList(City *from, City *to, Record *record, size_t *length) {
	size_t bufferSize = BUFFER_START_LENGTH * sizeof(City *);
	City *current = to, **buffer = malloc(bufferSize);
	for (*length = 0; current != from; current = record->prev[current->id]) {
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
	buffer[*length] = from;
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

City **shortestPath(City *from, City *to, CityMap *map) {
	City *current;
	Heap *queue;
	Record *record = recordMake(map->size);
	size_t length;
	current = from;
	if (!record)
		return NULL;
	record->prev[from->id] = from;
	queue = queueInit();
	if (!queue) {
		recordFree(&record);
		return NULL;
	}
	for (size_t distance = 0; current != to;) {
		assert(current != NULL);
		takeCity(queue, current, distance, record);
		if (queueEmpty(queue))
			return NULL;
		current = queuePop(queue, &distance);
	}
	return reverse(makeList(from, to, record, &length), length);
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
		ans->size = size;
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
	city->roadCount = 1;
	city->roadMax = 8;
}

void takeCity(Heap *queue, City *current, size_t distance, Record *record) {
	for (size_t i = 0; i < current->roadCount; ++i) {
		Road *r = current->roads[i];
		City *nextCity = (r->city1 != current ? r->city1 : r->city2);
		record->seen[current->id] = true;
		if (record->seen[nextCity->id])
			continue;
		record->prev[nextCity->id] = current;
		if (r->city1 != current)
			queuePush(queue, r->city1, distance + r->length);
		else
			queuePush(queue, r->city2, distance + r->length);
	}
}

void cityDestroy(City **city) {
	City *p = *city;
	free(p->name);
	free(p->roads);
	free(p);
	*city = NULL;
}

void debug(CityMap *m, Trie *t, const char *x, const char *y) {
	City *c1 = trieFind(t, x);
	City *c2 = trieFind(t, y);
	City **res = shortestPath(c1, c2, m);
}