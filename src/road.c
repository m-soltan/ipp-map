#include "road.h"

static bool initTrunks(Road *road, size_t length);

bool roadReserve(Road *road, size_t length) {
	assert(length < 1001);
	if (road->routes == NULL)
		return initTrunks(road, length);
	if (length > road->routeMax) {
		size_t newMax = road->routeMax, *tmp;
		while (newMax < length)
			newMax *= 2;
		tmp = realloc(road->routes, newMax * sizeof(size_t));
		if (tmp == NULL)
			return false;
		road->routes = tmp;
	}
	return true;
}

size_t roadGetFree(const Road *road) {
	return road->routeMax - road->routeCount;
}

unsigned roadGetLength(const Road *road) {
	return road->length;
}

size_t roadRouteCount(const Road *road) {
	return road->routeCount;
}

City *roadGetOther(Road *road, City *city) {
	if (road->city1 == city)
		return road->city2;
	assert(road->city1 < city);
	return road->city1;
}

// auxiliary function definitons
bool initTrunks(Road *road, size_t length) {
	assert(length > 0);
	road->routes = malloc(length * sizeof(size_t));
	road->routeMax = (road->routes != NULL ? 0 : length);
	return road->routeMax;
}
