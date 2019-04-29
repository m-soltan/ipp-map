#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "city.h"
#include "trunk.h"

struct Trunk {
	size_t length, id;
	City **cities;
};
// auxiliary function declarations
bool trunkAdjust(Trunk *t);
size_t trunkDescriptionLength(const Trunk *trunk);
Trunk *trunkJoin(Trunk *trunk, Trunk *extension);

// linked function definitions
bool trunkHasCity(const Trunk *trunk, const City *city) {
	for (size_t i = 0; i < trunk->length; ++i) {
		if (trunk->cities[i] == city)
			return true;
	}
	return false;
}

const char *trunkDescription(const Trunk *trunk) {
	char *ans, *temp;
	size_t size = trunkDescriptionLength(trunk);
	temp = ans = malloc(size);
	if (ans) {
		temp += sprintf(temp, "%zu;", trunk->id);
		assert(temp <= strlen("999;") + ans);
		cityGetName(temp, trunk->cities[0]);
		temp += cityGetNameLength(trunk->cities[0]);
		for (size_t i = 1; i < trunk->length; ++i) {
			temp += roadWrite(temp, trunk->cities[i - 1], trunk->cities[i]);
		}
	}
	return ans;
}

unsigned *trunkBlock(Trunk *trunk) {
	unsigned *ans;
	size_t size = (trunk->length - 1) * sizeof(unsigned);
	ans = malloc(size);
	if (ans) {
		for (size_t i = 0, j = 1; j < trunk->length; ++i, ++j) {
			ans[i] = roadBlock(trunk->cities[i], trunk->cities[j]);
		}
	}
	return ans;
}

void trunkAttach(Trunk *trunk) {
	for (size_t i = 1; i < trunk->length; ++i) {
		Road *r = roadFind(trunk->cities[i - 1], trunk->cities[i]);
		roadTrunkAdd(r, trunk->id);
	}
}

Trunk *trunkBuild(City *from, City *to, CityMap *m, unsigned number) {
	bool adjustSuccess;
	Trunk *ans = malloc(sizeof(Trunk));
	if (ans) {
		ans->cities = cityPath(from, to, m, &ans->length);
		ans->id = number;
		if (ans->cities) {
			adjustSuccess = trunkAdjust(ans);
			if (adjustSuccess)
				return ans;
			free(ans->cities);
		}
		free(ans);
	}
	return NULL;
}

Trunk *trunkDetour(CityMap *cMap, Trunk *trunk) {
	bool adjustSuccess;
	Trunk *ans = malloc(sizeof(Trunk));
	City *from = trunk->cities[0], *to = trunk->cities[trunk->length - 1];
	if (ans) {
		ans->cities = cityPath(from, to, cMap, &ans->length);
		ans->id = trunk->id;
		if (ans->cities) {
			adjustSuccess = trunkAdjust(ans);
			if (adjustSuccess)
				return ans;
			free(ans->cities);
		}
		free(ans);
	}
	return NULL;
}

Trunk *trunkExtend(CityMap *cityMap, Trunk *trunk, City *city) {
	size_t last = trunk->length - 1, len1, len2;
	Trunk *extension1, *extension2;
	extension1 = trunkBuild(city, trunk->cities[0], cityMap, trunk->id);
	extension2 = trunkBuild(trunk->cities[last], city, cityMap, trunk->id);
	if (extension1 == NULL && extension2 == NULL)
		return NULL;
	if (extension1 == NULL || extension2 == NULL) {
		return trunkJoin(trunk, (extension1 == NULL ? extension2 : extension1));
	}
	len1 = extension1->length;
	len2 = extension2->length;
	free(len1 > len2 ? extension1 : extension2);
	return trunkJoin(trunk, (len1 > len2 ? extension2 : extension1));
}

void trunkFree(Trunk **pTrunk) {
	Trunk *trunk = *pTrunk;
	*pTrunk = NULL;
	free(trunk->cities);
	free(trunk);
}

void trunkUnblock(Trunk *trunk, unsigned *lengths) {
	for (size_t i = 0, j = 1; j < trunk->length; ++i, ++j) {
		Road *r = roadFind(trunk->cities[i], trunk->cities[j]);
		roadUnblock(r, lengths[i]);
	}
}

// auxiliary function definitions
bool trunkAdjust(Trunk *t) {
	for (size_t i = 1; i < t->length; ++i) {
		bool adjustSuccess = roadAdjust(t->cities[i - 1], t->cities[i]);
		if (!adjustSuccess) {
			return false;
		}
	}
	return true;
}

size_t trunkDescriptionLength(const Trunk *trunk) {
	size_t ans = strlen("999;");
	for (size_t i = 0; i < trunk->length; ++i) {
		ans += cityGetNameLength(trunk->cities[i]) + strlen(";");
		ans += 3 * sizeof(unsigned) + strlen(";");
		ans += 3 * sizeof(int) + strlen(";");
	}
	return ans;
}

Trunk *trunkJoin(Trunk *trunk, Trunk *extension) {
	City *first2, *last1;
	Trunk *ans = malloc(sizeof(Trunk)), *prefix, *suffix;
	if (ans) {
		ans->length = trunk->length + extension->length - 1;
		ans->cities = malloc(ans->length * sizeof(City *));
		if (ans->cities) {
			first2 = extension->cities[0];
			last1 = trunk->cities[trunk->length - 1];
			prefix = (last1 == first2 ? trunk : extension);
			suffix = (last1 == first2 ? extension : trunk);
			size_t i = 0;
			for (i = 0; i < prefix->length; ++i)
				ans->cities[i] = prefix->cities[i];
			for (size_t j = 1; j < suffix->length; ++j)
				ans->cities[i + j - 1] = suffix->cities[j];
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