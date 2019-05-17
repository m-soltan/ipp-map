#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "city.h"
#include "city_map.h"
#include "queue.h"
#include "road.h"

// TODO: remove
#include "test.h"
//int counter = 0;
// until here

#define INIT_ROAD_MAX 16

typedef struct QPosition QPosition;
typedef struct Record Record;

struct City {
	bool blocked;
	char pad[sizeof(size_t) - sizeof(bool)];
	size_t id, nameSize, roadCount, roadMax;
	char *name;
	Road **roads;
};

struct QPosition {
	City *city;
	int minYear;
	char pad[sizeof(size_t) - sizeof(int)];
	size_t distance;
};

struct Record {
	bool *seen;
	Road **roads;
};

static bool isUnique(Heap *queue, City *to, int minYear, size_t d1);

static bool writePath(Heap *queue, QPosition position, City *from, City *to, Record *record);
static City initFields(const CityMap *cityMap, const char *name);
static QPosition pop(Heap *queue, Road **road);
static Road **makeList(City *from, City *to, Record *record, size_t *length);
static Record *recordMake(const CityMap *cityMap);
static size_t pathLength(const Record *record, City *from, City *to);
static void addRoad(City *city, Road *road);

static void recordFree(Record **pRecord);
static void visit(Heap *queue, QPosition position, Record *record);

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
	assert(city->roadCount < city->roadMax);
	city->roads[city->roadCount] = road;
	++city->roadCount;
}

size_t cityGetRoadCount(const City *city) {
	return city->roadCount;
}

//out of memory errors set length to 0, lack of path to SIZE_MAX
Road **cityPath(City *from, City *to, CityMap *cityMap, size_t *length) {
	*length = 0;
	Heap *queue;
	Record *record;
	Road **ans = NULL;
	record = recordMake(cityMap);
	if (!record)
		return NULL;
	record->roads[from->id] = NULL;
	queue = queueInit();
	if (queue) {
		QPosition position = (QPosition) {
				.city = from,
				.minYear = INT16_MAX,
				.distance = 0};
		bool writeResult = writePath(queue, position, from, to, record);
		if (!writeResult) {
			queueDestroy(&queue);
			recordFree(&record);
			*length = SIZE_MAX;
			return NULL;
		}
		if (isUnique(queue, to, position.minYear, position.distance))
			ans = makeList(from, to, record, length);
		queueDestroy(&queue);
	}
	recordFree(&record);
	return ans;
}

bool cityMakeSpace(City *city) {
	assert(city->roadMax >= city->roadCount && city->roadMax > 0);
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
	return city->nameSize - 1;
}

size_t cityGetName(char *dest, const City *city) {
	strncpy(dest, city->name, city->nameSize);
	return city->nameSize;
}

void cityDetachLast(City *city) {
	assert(cityGetRoadCount(city) > 0);
	Road *r = city->roads[city->roadCount - 1];
	roadDetach(r, city);
}

City *cityInit(CityMap *cityMap, const char *name, Road *road) {
	assert(name != NULL);
	City **pCity = cityMapAdd(cityMap);
	if (pCity) {
		City *ans;
		*pCity = ans = malloc(sizeof(City));
		if (ans) {
			*ans = initFields(cityMap, name);
			if (ans->roads) {
				assert(ans->name);
				assert(cityMapIsLast(cityMap, pCity));
				addRoad(ans, road);
				return ans;
			}
			free(ans);
		}
		cityMapRemove(cityMap);
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

Road *roadFind(City *city1, City *city2) {
	if (city1 == city2)
		return NULL;
	if (city1->roadCount > city2->roadCount)
		return roadFind(city2, city1);
	for (size_t i = 0; i < city1->roadCount; ++i) {
		if (roadHasCity(city1->roads[i], city2))
			return city1->roads[i];
	}
	return NULL;
}

// auxiliary function definitions
static Road **makeList(City *from, City *to, Record *record, size_t *length) {
	*length = pathLength(record, from, to);
	Road **buffer = malloc(*length * sizeof(Road *));
	if (buffer) {
		for (size_t i = *length; i > 0; --i) {
			City *city1, *city2;
			assert(to);
			buffer[i - 1] = record->roads[to->id];
			roadGetCities(buffer[i - 1], &city1, &city2);
			to = (city1 != to ? city1 : city2);
		}
		return buffer;
	}
	*length = 0;
	return NULL;
}

static Record *recordMake(const CityMap *cityMap) {
	Record *ans = malloc(sizeof(Record));
	size_t size = cityMapGetLength(cityMap);
	if (ans) {
		ans->seen = calloc(size, sizeof(bool));
		if (ans->seen) {
			ans->roads = malloc(size * sizeof(Road *));
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

static void visit(Heap *queue, QPosition position, Record *record) {
	City *current = position.city;
	for (size_t i = 0; i < current->roadCount; ++i) {
		Road *r = current->roads[i];
		City *city1, *city2;
		roadGetCities(r, &city1, &city2);
		assert(!current->blocked);
		bool blocked = city1->blocked || city2->blocked;
		if (!blocked && roadGetLength(r) != (unsigned) -1) {
			int minYear = roadGetYear(r);
			if (position.minYear < minYear)
				minYear = position.minYear;
			City *nextCity = (city1 != current ? city1 : city2);
			record->seen[current->id] = true;
			if (record->seen[nextCity->id])
				continue;
			unsigned length = roadGetLength(r);
			queuePush(queue, r, nextCity, position.distance + length, minYear);
		}
	}
}

size_t pathLength(const Record *record, City *from, City *to) {
	City *city1, *city2, *current, *prev;
	size_t ans = 0;
	for (current = to, prev = NULL; prev != from; current = prev) {
		roadGetCities(record->roads[current->id], &city1, &city2);
		assert(city1 == current || city2 == current);
		if (city1 == current)
			prev = city2;
		else
			prev = city1;
		++ans;
	}
	return ans;
}

/* TODO: check may miss a case when the path is not 1 - 1
*  and there is another city at the same distance from start
*/
bool isUnique(Heap *queue, City *to, int minYear, size_t d1) {
	City *check = NULL;
	int minYear2 = INT16_MAX;
	size_t d2 = SIZE_MAX;
	if (!queueEmpty(queue))
		queuePop(queue, &d2, &minYear2, &check);
	return check != to || d2 != d1 || minYear2 != minYear;
}

bool writePath(Heap *queue, QPosition position, City *from, City *to, Record *record) {
	for (position.city = from; position.city != to;) {
		Road *last;
		assert(position.city != NULL);
		visit(queue, position, record);
		if (queueEmpty(queue)) {
			return false;
		}
		position = pop(queue, &last);
		record->roads[position.city->id] = last;
	}
	return true;
}

static QPosition pop(Heap *queue, Road **road) {
	City *city;
	size_t d;
	int minYear;
	*road = queuePop(queue, &d, &minYear, &city);
	return (QPosition) {.city = city, .distance = d, .minYear = minYear};
}

City initFields(const CityMap *cityMap, const char *name) {
	City ans = (City) {
		.blocked = false,
		.id = cityMapGetLength(cityMap) - 1,
		.nameSize = 1 + strlen(name),
		.roadCount = 0,
		.roadMax = INIT_ROAD_MAX
	};
	ans.name = malloc(ans.nameSize);
	if (ans.name) {
		ans.roads = malloc(ans.roadMax * sizeof(Road *));
		if (ans.roads) {
			strcpy(ans.name, name);
			return ans;
		}
		free(ans.name);
		ans.name = NULL;
	}
	return ans;
}
