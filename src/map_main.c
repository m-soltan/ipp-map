#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "map.h"

Map* m;
char* buff;
unsigned long lineNr;

#define ERROR(x) { \
	fprintf(stderr, "Line %lu: %s\n", lineNr, x); \
	for (int i = 0; i < argc; i++) free(argv[i]); \
	continue; \
}

#define ERRORN(x) do { \
	deleteMap(m); \
	free(buff); \
	fprintf(stderr, "Line %lu: %s\n", lineNr, x); \
	exit(1); \
} while (0);

#define NUM "Wrong number of arguments"

void bp() {

}

unsigned toUnsigned(const char* str) {
	size_t len = strlen(str);
	unsigned result = 0;
	for (size_t i = 0; i < len; i++) {
		if (!isdigit(str[i])) ERRORN("Not a number");
		result = 10 * result + str[i] - '0';
	}
	return result;
}

int toSigned(const char* str) {
	size_t len = strlen(str);
	int result = 0, sign = 1;
	for (size_t i = 0; i < len; i++) {
		if (!isdigit(str[i])) {
			if (i == 0 && str[i] == '-') sign = -1;
			else ERRORN("Not a number");
		} else {
			result = 10 * result + str[i] - '0';
		}
	}
	//fprintf(stderr, "%s %d\n", str, sign * result);
	return sign * result;
}

int main() {
	m = newMap();
	assert(m);
	size_t n = 0;

	for (lineNr = 1;; lineNr++) {
		ssize_t size = getline(&buff, &n, stdin);
		if (size == -1) break;
		int argc = 0;
		char* argv[5];
		size_t last = 0;
		if (size > 0 && buff[0] == '#') continue;
		for (int i = 0; i < size; i++) {
			if (buff[i] == ';' || buff[i] == '\n') {
				if (i == 0 && buff[i] == '\n') break;
				if (argc == 5) ERROR("Too many arguments");
				argv[argc] = malloc(i - last + 1);
				if (argv[argc] == NULL) ERROR("Memory error");
				memcpy(argv[argc], buff + last, i - last);
				argv[argc++][i - last] = 0;
				last = i + 1;
			}
		}

		if (argc == 0) continue;
		bool result = false;
		bool wypisac = true;

		if (strcmp(argv[0], "addRoad") == 0) {
			if (argc != 5) ERROR(NUM);
			result = addRoad(m, argv[1], argv[2], toUnsigned(argv[3]), toSigned(argv[4]));
		} else if (strcmp(argv[0], "repairRoad") == 0) {
			if (argc != 4) ERROR(NUM);
			result = repairRoad(m, argv[1], argv[2], toSigned(argv[3]));
		} else if (strcmp(argv[0], "newRoute") == 0) {
			if (argc != 4) ERROR(NUM);
			result = newRoute(m, toUnsigned(argv[1]), argv[2], argv[3]);
		} else if (strcmp(argv[0], "extendRoute") == 0) {
			if (argc != 3) ERROR(NUM);
			if (lineNr == 177) bp();
			result = extendRoute(m, toUnsigned(argv[1]), argv[2]);
		} else if (strcmp(argv[0], "removeRoad") == 0) {
			if (argc != 3) ERROR(NUM);
//			if (lineNr == 231) bp();
			result = removeRoad(m, argv[1], argv[2]);
		} else if (strcmp(argv[0], "getRouteDescription") == 0) {
//			if (lineNr == 318) bp();
			if (argc != 2) ERROR(NUM);
			const char* str = getRouteDescription(m, toUnsigned(argv[1]));
			if (str == NULL) ERROR("Memory error");
			printf("%lu: %s\n", lineNr, str);
			free((void*) str);
			wypisac = false;
		} else {
			bp();
		}
		if (wypisac) printf("%lu: %s\n", lineNr, result ? "TAK" : "NIE");
		for (int i = 0; i < argc; i++) {
			free(argv[i]);
		}
	}

	free(buff);
	deleteMap(m);
	return 0;
}