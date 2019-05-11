#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "city.h"
#include "city_map.h"
#include "queue.h"
#include "road.h"

typedef struct QPosition QPosition;
typedef struct Record Record;
typedef struct RecordOld RecordOld;

struct City {
	bool blocked;
	char pad[sizeof(size_t) - sizeof(bool)];
	size_t id, nameLength, roadCount, roadMax;
	char *name;
	Road **roads;
};

struct QPosition {
	size_t distance;
	int minYear;
};

struct Record {
	bool *seen;
	Road **roads;
};

struct RecordOld {
	bool *seen;
	City **prev;
};

static bool isUnique(Heap *queue, City *to, int minYear, size_t d1);
static Road **makeList(City *from, City *to, Record *record, size_t *length);
static Record *recordMake(const CityMap *cityMap);
static size_t pathLength(const Record *r, City *from, City *to);
static void addRoad(City *city, Road *road);
static void initFields(City *city);
static void recordFree(Record **pRecord);
static void visit(Heap *queue, City *current, size_t distance, Record *record, int minYear);

bool cityConnectRoad(City *city, Road *road) {
	if (!cityMakeSpace(city))
		return false;
	addRoad(city, road);
	return true;
}

bool cityMakeRoad(City *city1, City *city2, Road *road) {
	if (!cityMakeSpace(city1) || !cityMakeSpace(city2))
		return false;
	addRoad(city1, road);
	addRoad(city2, road);
	return true;
}

void addRoad(City *city, Road *road) {
	city->roads[city->roadCount] = road;
	++city->roadCount;
}

size_t cityGetRoadCount(const City *city) {
	return city->roadCount;
}

// TODO: distinguish no route and no memory
Road **cityPath(City *from, City *to, CityMap *cityMap, size_t *length) {
	Heap *queue = queueInit();
	Record *record = recordMake(cityMap);
	Road **ans = NULL;
	if (!record)
		return NULL;
	record->roads[from->id] = NULL;
	if (queue) {
		City *current = from;
		int minYear = INT16_MAX;
		size_t d1 = 0;
		for (; current != to;) {
			Road *last;
			assert(current != NULL);
			visit(queue, current, d1, record, minYear);
			if (queueEmpty(queue)) {
				queueDestroy(&queue);
				recordFree(&record);
				return NULL;
			}
			last = queuePop(queue, &d1, &minYear, &current);
			record->roads[current->id] = last;
		}
		if (isUnique(queue, to, minYear, d1))
			ans = makeList(from, to, record, length);
		else
			ans = NULL;
		queueDestroy(&queue);
	}
	recordFree(&record);
	return ans;
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

size_t cityGetNameLength(const City *city) {
	return city->nameLength - 1;
}

size_t cityGetName(char *dest, const City *city) {
	strncpy(dest, city->name, city->nameLength);
	return city->nameLength;
}

void cityDetachLast(City *city) {
	assert(cityGetRoadCount(city) > 0);
	Road *r = city->roads[city->roadCount - 1];
	cityDetach(city, r);
	if (r->city1 == city)
		r->city1 = NULL;
	else
		r->city2 = NULL;
	if (r->city1 == NULL && r->city2 == NULL) {
		free(r->routes);
		free(r);
	}
}

City *cityInit(CityMap *map, const char *name, Road *road) {
	City **ans, *c;
	assert(name != NULL);
	ans = cityMapAdd(map);
//	TODO: if (ans == NULL) {}
	*ans = calloc(1, sizeof(City));
	c = *ans;
	c->id = cityMapGetLength(map);
	c->nameLength = 1 + strlen(name);
	initFields(c);
	c->roads = malloc(c->roadMax * sizeof(Road));
	if (c->roads) {
		c->roads[0] = road;
		c->name = malloc(c->nameLength);
		if (c->name) {
			strcpy(c->name, name);
			c->name[c->nameLength - 1] = '\0';
			return c;
		}
		free(c->roads);
	}
	cityMapRemove(map);
	return NULL;
}

Road *cityRoadTo(const City *city1, const City *city2) {
	for (size_t i = 0; i < city1->roadCount; ++i) {
		if (city1->roads[i]->city2 == city2)
			return city1->roads[i];
	}
	return NULL;
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

void cityBlock(City *city) {
	city->blocked = true;
}

void cityUnblock(City *city) {
	city->blocked = false;
}

void cityDestroy(City **city, CityMap *cityMap) {
	assert(cityMapIsLast(cityMap, city));
	City *tmp = *city;
	free(tmp->name);
	free(tmp->roads);
	free(*city);
	*city = NULL;
}


// auxiliary function definitions
static Road **makeList(City *from, City *to, Record *record, size_t *length) {
	*length = pathLength(record, from, to);
	Road **buffer = malloc(*length * sizeof(Road *));
	for (size_t i = *length; i > 0; --i) {
		City *city1, *city2;
		assert(to);
		buffer[i - 1] = record->roads[to->id];
		roadGetCities(buffer[i - 1], &city1, &city2);
		to = (city1 != to ? city1 : city2);
	}
	return buffer;
}

static Record *recordMake(const CityMap *cityMap) {
	Record *ans = malloc(sizeof(RecordOld));
	size_t size = cityMapGetLength(cityMap);
	if (ans) {
		ans->seen = calloc(size, sizeof(bool));
		if (ans->seen) {
			ans->roads = malloc((size - 1) * sizeof(Road *));
			if (ans->roads)
				return ans;
		}
		free(ans);
	}
	return NULL;
}

static void recordFree(Record **pRecord) {
	Record *record = *pRecord;
	free(record->roads);
	free(record->seen);
	free(record);
	*pRecord = NULL;
}

static void visit(Heap *queue, City *current, size_t distance, Record *record, int minYear) {
	for (size_t i = 0; i < current->roadCount; ++i) {
		Road *r = current->roads[i];
		City *city1, *city2;
		roadGetCities(r, &city1, &city2);
		assert(!current->blocked);
		if (city1->blocked || city2->blocked)
			continue;
		minYear = (roadGetYear(r) < minYear ? roadGetYear(r) : minYear);
		City *nextCity = (city1 != current ? city1 : city2);
		record->seen[current->id] = true;
		if (record->seen[nextCity->id])
			continue;
		unsigned length = roadGetLength(r);
		queuePush(queue, r, nextCity, distance + length, minYear);
	}
}

static void initFields(City *city) {
	city->blocked = false;
	city->roadCount = 1;
	city->roadMax = 8;
}

size_t pathLength(const Record *r, City *from, City *to) {
	City *city1, *city2, *current, *prev;
	size_t ans = 0;
	for (current = to, prev = NULL; prev != from; current = prev) {
		roadGetCities(r->roads[current->id], &city1, &city2);
		prev = (city1 != current ? city1 : city2);
		++ans;
	}
	return ans;
}

/* TODO: check may miss a case when the path is not 1 - 1,
*  but there is another city at the same distance from start
*/
bool isUnique(Heap *queue, City *to, int minYear, size_t d1) {
	City *check = NULL;
	int minYear2 = INT16_MAX;
	size_t d2 = SIZE_MAX;
	if (!queueEmpty(queue))
		queuePop(queue, &d2, &minYear2, &check);
	return check != to || d2 != d1 || minYear2 != minYear;
}
