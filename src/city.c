#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "city.h"
#include "city_map.h"
#include "queue.h"
#include "road.h"

#define INIT_ROAD_MAX 16

typedef struct QPosition QPosition;
typedef struct Record Record;

struct City {
	bool blocked;
	bool pad[7];
	char *name;
	size_t id, nameSize, roadCount, roadMax;
	Road **roads;
};

struct QPosition {
	City *city;
	int minYear;
	int pad;
	size_t distance;
};

struct Record {
	bool *repeated;
	size_t *d;
	int *years;
	Road **roads;
};

static bool initFields(City *city, CityInfo info, size_t id);
static bool isUnique(Heap *queue, City *to, int minYear, size_t d1, bool repeated);
static bool makeSpace(City *city);
static bool writePath(Heap *queue, QPosition *position, City *from, City *to, Record *record);
static size_t pathLength(const Record *record, City *start, City *finish);
static void addRoad(City *city, Road *road);
static void markVisited(Record *record, size_t cityId, size_t distance, int year);
static void recordFree(Record **pRecord);
static void visit(Heap *queue, QPosition position, Record *record);
static City *add(CityInfo info);
static City *init(CityInfo info, size_t id);
static QPosition pop(Heap *queue, Road **road);
static Road **makeList(City *from, City *to, Record *record, size_t *length);
static Record *recordMake(const CityMap *cityMap);

bool cityConnectRoad(City *city, Road *road) {
	if (!makeSpace(city))
		return false;
	addRoad(city, road);
	return true;
}

bool cityMakeRoad(City *city1, City *city2, Road *road) {
	if (!makeSpace(city1) || !makeSpace(city2))
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
		bool writeResult = writePath(queue, &position, from, to, record);
		if (!writeResult) {
			queueDestroy(&queue);
			recordFree(&record);
			*length = SIZE_MAX;
			return NULL;
		}
		bool repeated = record->repeated[to->id];
		if (isUnique(queue, to, position.minYear, position.distance, repeated)) {
			ans = makeList(from, to, record, length);
		} else {
			*length = SIZE_MAX;
		}
		queueDestroy(&queue);
	}
	recordFree(&record);
	return ans;
}

size_t cityGetNameLength(const City *city) {
	return city->nameSize - 1;
}

size_t cityCopyName(char *dest, const City *city) {
	strncpy(dest, city->name, city->nameSize);
	return city->nameSize;
}

void cityDetachLast(City *city) {
	assert(cityGetRoadCount(city) > 0);
	Road *r = city->roads[city->roadCount - 1];
	roadDetach(r, city);
}

City *cityAdd(CityMap *cityMap, const char *name, Road *road) {
	City *ans = add((CityInfo) {.cityMap = cityMap, .name = name});
	addRoad(ans, road);
	return ans;
}

void cityDetach(City *city, const Road *road) {
	Road **last = &city->roads[city->roadCount - 1], *temp = *last;
	for (size_t i = 0; i < city->roadCount; ++i) {
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

void cityDestroy(City **pCity) {
	City *city = *pCity;
	free(city->name);
	free(city->roads);
	free(*pCity);
	*pCity = NULL;
}

Road *cityFindRoad(City *city1, City *city2) {
	if (city1 && city2) {
		if (city1 == city2)
			return NULL;
		if (city1->roadCount > city2->roadCount)
			return cityFindRoad(city2, city1);
		const size_t count = cityGetRoadCount(city1);
		for (size_t i = 0; i < count; ++i) {
			if (roadHasCity(city1->roads[i], city2))
				return city1->roads[i];
		}
	}
	return NULL;
}

const char * cityGetName(const City *city) {
	return city->name;
}

City *cityDecoy() {
	City *ans = malloc(sizeof(City));
	if (ans) {
		*ans = (City) {
			.blocked = 3,
			.name = NULL,
			.id = SIZE_MAX,
			.nameSize = SIZE_MAX,
			.roadCount = SIZE_MAX,
			.roadMax = 0,
			.roads = NULL,
		};
	}
	return ans;
}

static Road **makeList(City *from, City *to, Record *record, size_t *length) {
	*length = pathLength(record, from, to);
	Road **buffer = malloc(*length * sizeof(Road *));
	if (buffer) {
		for (size_t i = *length; i > 0; --i) {
			if (record->repeated[to->id]) {
				free(buffer);
				*length = SIZE_MAX;
				return NULL;
			}
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
	size_t size = cityMapGetLength(cityMap);
	Record *ans = malloc(sizeof(Record));
	if (ans) {
		ans->repeated = calloc(size, sizeof(bool));
		if (ans->repeated) {
			ans->d = calloc(size, sizeof(size_t));
			ans->years = calloc(size, sizeof(size_t));
			if (ans->years) {
				if (ans->d) {
					ans->roads = calloc(size, sizeof(Road *));
					if (ans->roads)
						return ans;
					free(ans->d);
				}
				free(ans->years);
			}
			free(ans->repeated);
		}
		free(ans);
	}
	return NULL;
}

static void recordFree(Record **pRecord) {
	Record *record = *pRecord;
	free(record->d);
	free(record->repeated);
	free(record->roads);
	free(record->years);
	free(record);
	*pRecord = NULL;
}

static void visit(Heap *queue, QPosition position, Record *record) {
	City *const current = position.city;
	assert(!current->blocked);
	markVisited(record, current->id, position.distance, position.minYear);
	for (size_t i = 0; i < current->roadCount; ++i) {
		Road *r = current->roads[i];
		City *city1, *city2, *nextCity;
		roadGetCities(r, &city1, &city2);
		nextCity = (city1 != current ? city1 : city2);
		if (!nextCity->blocked && roadGetLength(r) != (unsigned) -1) {
			int minYear = roadGetYear(r);
			if (position.minYear < minYear)
				minYear = position.minYear;
			if (record->d[nextCity->id])
				continue;
			size_t distance = position.distance + roadGetLength(r);
			queuePush(queue, r, nextCity, distance, minYear);
		}
	}
}

size_t pathLength(const Record *record, City *start, City *finish) {
	City *city1, *city2, *current = finish, *nextCity = NULL;
	for (size_t ans = 1; true; ++ans) {
		assert(current != start);
		roadGetCities(record->roads[current->id], &city1, &city2);
		if (city1 == current) {
			nextCity = city2;
		} else if (city2 == current) {
			nextCity = city1;
		} else {
			assert(false);
			return 0;
		}
		if (nextCity == start)
			return ans;
		current = nextCity;
	}
}

bool isUnique(Heap *queue, City *to, int minYear, size_t d1, bool repeated) {
	City *check = NULL;
	int minYear2 = INT16_MAX;
	size_t d2 = SIZE_MAX;
	if (repeated)
		return false;
	while (!queueEmpty(queue)) {
		queuePop(queue, &d2, &minYear2, &check);
		if (check != to || d2 != d1 || minYear2 != minYear)
			continue;
		return false;
	}
	return true;
}

bool writePath(Heap *queue, QPosition *position, City *from, City *to, Record *record) {
	for (position->city = from; position->city != to;) {
		Road *last;
		assert(position->city != NULL);
		visit(queue, *position, record);
		if (queueEmpty(queue))
			return false;
		*position = pop(queue, &last);
		if (record->d[position->city->id])
			continue;
		record->roads[position->city->id] = last;
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

static bool initFields(City *city, CityInfo info, size_t id) {
	size_t nameSize = 1 + strlen(info.name);
	*city = (City) {
		.blocked = false,
		.id = id,
		.name = malloc(nameSize),
		.nameSize = nameSize,
		.roadCount = 0,
		.roadMax = INIT_ROAD_MAX,
	};
	if (city->name) {
		city->roads = malloc(city->roadMax * sizeof(Road *));
		if (city->roads) {
			strcpy(city->name, info.name);
			return true;
		}
		free(city->name);
		city->name = NULL;
	}
	return false;
}

/* sets the current id as visited, checks if it has already been visited before
 * with the same distance and min. year
 * if yes, records a repetition
 */
static void markVisited(Record *record, size_t cityId, size_t distance, int year) {
	if (record->d[cityId] == 0) {
		record->d[cityId] = distance;
		record->years[cityId] = year;
	} else if (record->d[cityId] == distance && record->years[cityId] == year) {
		record->repeated[cityId] = true;
	}
}

static bool makeSpace(City *city) {
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

static City *init(CityInfo info, size_t id) {
	City *ans = malloc(sizeof(City));
	if (ans) {
		bool success = initFields(ans, info, id);
		if (success) {
			assert(ans->name);
			return ans;
		}
		free(ans);
	}
	return NULL;
}

static City *add(CityInfo info) {
	assert(info.name != NULL);
	City *ans = cityMapAddCity(info, init);
	if (ans) {
		return ans;
	}
	return NULL;
}
