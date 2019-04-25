#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "city.h"
#include "trunk.h"

struct City {
	size_t nameLen, roadCount, roadMax;
	char *name;
	Road **roads;
};

struct Road {
	City *city1, *city2;
	List *routes;
	unsigned length;
	int year;
};

// auxiliary function declarations
bool roadInsert(Trie *t, Road **road);
void cityDestroy(City **city);

// linked function definitions
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
	size_t index;
	if (!cityMakeSpace(city))
		return false;
	index = city->roadCount;
	++city->roadCount;
	city->roads[index] = road;
	return true;
}

// TODO
bool roadDestroy(Road *road) {
	City *city1 = road->city1, *city2 = road->city2;
	cityDetach(city1, road);
	cityDetach(city2, road);
	return true;
}

bool roadExtend(Trie *t, City *city, RoadInfo info) {
	bool successAdd, successInsert;
	const char *str = (info.city1 ? info.city1 : info.city2);
	Road *r = malloc(sizeof(Road));
	if (r) {
		City *c = cityInit(r, str);
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
				return true;
			}
			--city1->roadCount;
		}
		free(r);
	}
	return false;
}

City *cityInit(Road *road, const char *name) {
	City *ans = malloc(sizeof(City));
	if (ans) {
		ans->roadMax = 8;
		ans->roads = malloc(ans->roadMax * sizeof(Road));
		if (ans->roads) {
			ans->roads[0] = road;
			ans->roadCount = 1;
			ans->nameLen = strlen(name);
			ans->name = malloc(ans->nameLen);
			if (ans->name) {
				strncpy(ans->name, name, ans->nameLen);
				return ans;
			}
			free(ans->roads);
		}
		free(ans);
	}
	return NULL;
}

bool roadInit(Trie *t, RoadInfo info) {
	Road *ans = malloc(sizeof(Road));
	if (!ans)
		return NULL;
	City *c1 = cityInit(ans, info.city1), *c2 = cityInit(ans, info.city2);
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

void cityDestroy(City **city) {
	City *p = *city;
	free(p->name);
	free(p->roads);
	free(p);
	*city = NULL;
}
