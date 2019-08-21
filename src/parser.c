#include <ctype.h>
#include <stdio.h>
#include "map.h"
#include "parser.h"

#define COMMENT_SYMBOL '#'
#define INIT_BUFFER_SIZE 2

typedef struct Addition Addition;
typedef struct Creation Creation;
typedef struct Extension Extension;
typedef struct Description Description;
typedef struct New New;
typedef struct RemRoad RemRoad;
typedef struct RemRoute RemRoute;
typedef struct Repair Repair;
typedef struct Word Word;

struct Addition {
	char *city1;
	char *city2;
	int builtYear;
	unsigned length;
};

struct Creation {
	unsigned routeId;
	unsigned pad;
	const char **cityNames;
	unsigned *roadLengths;
	int *builtYears;
	size_t length, lengthMax;

};

struct Description {
	unsigned routeId;
};

struct Extension {
	unsigned routeId;
	unsigned pad;
	char *city;
};

struct New {
	unsigned routeId;
	unsigned pad;
	char *city1, *city2;
};

struct RemRoad {
	char *city1;
	char *city2;
};

struct RemRoute {
	unsigned routeId;
	unsigned pad;
};

struct Repair {
	char *city1;
	char *city2;
	int repairYear;
	int pad;
};

struct Word {
	const char *str;
	size_t (*fun)(const char *);
};

static bool charIsLetter(char c);
static bool isAddition(const char *str);
static bool isComment(const char *str);
static bool isCreation(const char *str);
static bool isDescription(const char *str);
static bool isExtension(const char *str);
static bool isNew(const char *str);
static bool isRemRoad(const char *str);
static bool isRemRoute(const char *str);
static bool isRepair(const char *str);
static bool resize(Creation *c);
static const char *accept(Word *word);
static const char *acceptFirst(Word *word);
static const char *acceptNext(Word *word);
static const char *acceptPrefix(const char *str, const char *prefix);
static char *nextWord(void);
static int push(Creation *c, const char *cityName, long roadLength, long year);
static long nextLongInt(void);
static size_t isCityName(const char *str);
static size_t isLongInt(const char *str);
static void doAddition(Addition *ptr);
static void doCreation(Creation *ptr);
static void doExtension(Extension *ptr);
static void doDescription(Description *ptr);
static void doNew(New *ptr);
static void doRemRoad(RemRoad *ptr);
static void doRemRoute(RemRoute *ptr);
static void doRepair(Repair *ptr);
static void creationDestroy(Creation **pCreation);
static Addition *getAddition(char *str);
static Creation *getCreation(char *str);
static Description *getDescription(char *str);
static Extension *getExtension(char *str);
static New *getNew(char *str);
static RemRoad *getRemRoad(char *str);
static RemRoute *getRemRoute(char *str);
static Repair *getRepair(char *str);

static Map *globalMap = NULL;
static size_t lineNumber = 0;

bool setMap() {
	assert(globalMap == NULL);
	globalMap = newMap();
	return globalMap != NULL;
}

void writeError() {
	fprintf(stderr, "ERROR %zu\n", lineNumber);
}

char *getLine(void) {
	size_t bufferSize = INIT_BUFFER_SIZE;
	char *buffer = calloc(bufferSize, sizeof(char));
	size_t i = 0;
	++lineNumber;
	while (true) {
		int c = fgetc(stdin);
		if (c == '\n' || c == EOF)
			return buffer;
		if (i == bufferSize - 1) {
			bufferSize *= 2;
			char *tmp = realloc(buffer, bufferSize);
			if (tmp == NULL) {
				free(buffer);
				return NULL;
			}
			buffer = tmp;
		}
		buffer[i] = (char) c;
		buffer[i + 1] = '\0';
		++i;
	}
}

void parserRead(char *line) {
	// addRoad
	if (isAddition(line))
		doAddition(getAddition(line));
	// route literal
	else if (isCreation(line))
		doCreation(getCreation(line));
	// comment
	else if (isComment(line))
		return;
	// getRouteDescription
	else if (isDescription(line))
		doDescription(getDescription(line));
	// extendRoute
	else if (isExtension(line))
		doExtension(getExtension(line));
	// newRoute
	else if (isNew(line))
		doNew(getNew(line));
	// removeRoad
	else if (isRemRoad(line))
		doRemRoad(getRemRoad(line));
	// removeRoute
	else if (isRemRoute(line))
		doRemRoute(getRemRoute(line));
	// repairRoute
	else if (isRepair(line))
		doRepair(getRepair(line));
	else
		writeError();
	free(line);
}

void parserExamples() {
	assert(isAddition("addRoad;a;b;1;10"));
	assert(isComment("#"));
	assert(isDescription("getRouteDescription;10"));
	assert(isExtension("extendRoute;10;a"));
	assert(isNew("newRoute;10;a;b"));
	assert(isRemRoad("removeRoad;a;b"));
	assert(isRemRoute("removeRoute;10"));
	assert(isRepair("repairRoad;a;b;-1"));
}

int runParser(void) {
	bool success;
	success = setMap();
	if (!success)
		return OUT_OF_MEMORY;
	while (true) {
		int c = fgetc(stdin);
		if (c == EOF)
			return 0;
		else
			ungetc(c, stdin);
		char *line = getLine();
		if (line == NULL) {
			deleteMap(globalMap);
			return OUT_OF_MEMORY;
		}
		parserRead(line);
	}
}

static bool isAddition(const char *str) {
	str = acceptPrefix(str, "addRoad");
	for (size_t i = 0; i < 2; ++i)
		str = acceptNext(&(Word) {.str = str, .fun = isCityName});
	for (size_t i = 0; i < 2; ++i)
		str = acceptNext(&(Word) {.str = str, .fun = isLongInt});
	return str && str[0] == '\0';
}

static bool isComment(const char *str) {
	char c = str[0];
	return c == '\0' || c == COMMENT_SYMBOL;
}

static bool isCreation(const char *str) {
	str = acceptFirst(&(Word) {.str = str, .fun = isLongInt});
	for (size_t i = 0; true; ++i) {
		str = acceptNext(&(Word) {.str = str, .fun = isCityName});
		if (str == NULL)
			return false;
		if (str[0] != ';')
			break;
		for (int j = 0; j < 2; ++j)
			str = acceptNext(&(Word) {.str = str, .fun = isLongInt});
	}
	return str[0] == '\0';
}

static bool isRepair(const char *str) {
	str = acceptPrefix(str, "repairRoad");
	for (size_t i = 0; i < 2; ++i)
		str = acceptNext(&(Word) {.str = str, .fun = isCityName});
	str = acceptNext(&(Word) {.str = str, .fun = isLongInt});
	return str && str[0] == '\0';
}

static bool isDescription(const char *str) {
	str = acceptPrefix(str, "getRouteDescription");
	str = acceptNext(&(Word) {.str = str, .fun = isLongInt});
	return str && str[0] == '\0';
}

static void doAddition(Addition *ptr) {
	assert(globalMap != NULL);
	if (ptr) {
		bool success = addRoad(
				globalMap,
				ptr->city1,
				ptr->city2,
				ptr->length,
				ptr->builtYear);
		free(ptr->city1);
		free(ptr->city2);
		free(ptr);
		if (success)
			return;
	}
	writeError();
}

static Addition *getAddition(char *str) {
	Addition *ans = malloc(sizeof(Addition));
	if (ans) {
		strtok(str, ";");
		char *city1 = nextWord();
		char *city2 = nextWord();
		long builtYear = nextLongInt();
		long length = nextLongInt();
		*ans = (Addition) {
				.builtYear = (int) builtYear,
				.length = (unsigned) length};
		if (length == ans->length && builtYear == ans->builtYear) {
			ans->city1 = malloc(sizeof('\0') + strlen(city1));
			if (ans->city1) {
				ans->city2 = malloc(sizeof('\0') + strlen(city2));
				if (ans->city2) {
					strcpy(ans->city1, city1);
					strcpy(ans->city2, city2);
					return ans;
				}
				free(ans->city1);
			}
		}
		free(ans);
	}
	return NULL;
}

static long nextLongInt() {
	char *s = nextWord();
	if (s == NULL)
		return 0;
	return strtol(s, NULL, 10);
}

static char *nextWord() {
	return strtok(NULL, ";");
}

static size_t isCityName(const char *str) {
	for (size_t i = 0; true; ++i) {
		if (str[i] == ';' || str[i] == '\0')
			return i;
		if (!charIsLetter(str[i]))
			return 0;
	}
}

static bool charIsLetter(char c) {
	bool ans = true;
	ans = ans && c != ';';
	ans = ans && (c < '\0' || c > '\x1f');
	return ans;
}

static size_t isLongInt(const char *str) {
	if (str[0] == '0')
		return false;
	for (size_t i = 0; true; ++i) {
		if (str[i] == ';' || str[i] == '\0')
			return i;
		if (i == 0 && str[i] == '-')
			continue;
		if (!isdigit(str[i]))
			return 0;
	}
}

static Repair *getRepair(char *str) {
	Repair *ans = malloc(sizeof(Repair));
	if (ans) {
		strtok(str, ";");
		char *city1 = nextWord();
		char *city2 = nextWord();
		long repairYear = nextLongInt();
		*ans = (Repair) {
			.city1 = city1,
			.city2 = city2,
			.repairYear = (int) repairYear};
		if (repairYear == ans->repairYear)
			return ans;
		free(ans);
	}
	return NULL;
}

static void doRepair(Repair *ptr) {
	assert(globalMap != NULL);
	if (ptr) {
		bool success = repairRoad(
				globalMap,
				ptr->city1,
				ptr->city2,
				ptr->repairYear);
		if (success)
			return;
	}
	writeError();
}

static Description *getDescription(char *str) {
	Description *ans = malloc(sizeof(Description));
	if (ans) {
		strtok(str, ";");
		long routeId = nextLongInt();
		*ans = (Description) {.routeId = (unsigned) routeId};
		if (ans->routeId == routeId)
			return ans;
		free(ans);
	}
	return NULL;
}

static void doDescription(Description *ptr) {
	assert(globalMap != NULL);
	if (ptr) {
		char *success = routeDescriptionAux(globalMap, ptr->routeId);
		fprintf(stdout, "%s\n", success);
		free(ptr);
		if (success) {
			free(success);
			return;
		}
	}
	writeError();
}

static Creation *getCreation(char *str) {
	Creation *ans = malloc(sizeof(Creation));
	if (ans) {
		long routeId = strtol(strtok(str, ";"), NULL, 10);
		*ans = (Creation) {.routeId = (unsigned) routeId};
		if (ans->routeId == routeId) {
			for (bool stay = true; stay;) {
				int pushResult;
				char *cityName = nextWord();
				long roadLength = nextLongInt();
				long builtYear = nextLongInt();
				if (roadLength == 0 || builtYear == 0)
					stay = false;
				pushResult = push(ans, cityName, roadLength, builtYear);
				if (pushResult != 0) {
					creationDestroy(&ans);
					return NULL;
				}
			}
			return ans;
		}
		free(ans);
	}
	return NULL;
}

static int push(Creation *c, const char *cityName, long roadLength, long year) {
	bool lengthSuccess, yearSuccess;
	if (!resize(c))
		return OUT_OF_MEMORY;
	c->cityNames[c->length] = cityName;
	c->roadLengths[c->length] = (unsigned) roadLength;
	c->builtYears[c->length] = (int) year;
	lengthSuccess = c->roadLengths[c->length] == roadLength;
	yearSuccess = c->builtYears[c->length] == year;
	if (!lengthSuccess || !yearSuccess)
		return INVALID_ARG;
	++c->length;
	return 0;
}

static void creationDestroy(Creation **pCreation) {
	Creation *creation = *pCreation;
	*pCreation = NULL;
	if (creation->length > 0) {
		free(creation->cityNames);
		free(creation->builtYears);
		free(creation->roadLengths);
		free(creation);
	}
}

static bool resize(Creation *c) {
	const size_t initialLength = 32;
	size_t newLength;
	if (c->length < c->lengthMax)
		return true;
	if (c->lengthMax == 0)
		newLength = initialLength;
	else
		newLength = 2 * c->lengthMax;
	const char **tmpNames = realloc(c->cityNames, sizeof(char *) * newLength);
	if (tmpNames) {
		int *tmpYears = realloc(c->builtYears, sizeof(int) * newLength);
		if (tmpYears) {
			unsigned *tmpLengths;
			tmpLengths = realloc(c->roadLengths, sizeof(unsigned) * newLength);
			if (tmpLengths) {
				c->lengthMax = newLength;
				c->cityNames = tmpNames;
				c->builtYears = tmpYears;
				c->roadLengths = tmpLengths;
				return true;
			}
			free(tmpYears);
		}
		free(tmpNames);
	}
	creationDestroy(&c);
	return false;
}

static void doCreation(Creation *ptr) {
	assert(globalMap != NULL);
	if (ptr) {
		bool success;
		success = routeFromList(
				globalMap,
				ptr->routeId,
				ptr->cityNames,
				ptr->roadLengths,
				ptr->builtYears,
				ptr->length);
		free(ptr->cityNames);
		free(ptr->builtYears);
		free(ptr->roadLengths);
		free(ptr);
		if (success) {
			return;
		}
	}
	writeError();
}

static const char *accept(Word *word) {
	size_t length = word->fun(word->str);
	if (length > 0)
		return word->str + length;
	else
		return NULL;
}

static const char *acceptFirst(Word *word) {
	if (word->str == NULL)
		return NULL;
	return accept(word);
}

static const char *acceptNext(Word *word) {
	if (word->str == NULL || word->str[0] != ';')
		return NULL;
	++word->str;
	return accept(word);
}

static bool isExtension(const char *str) {
	str = acceptPrefix(str, "extendRoute");
	str = acceptNext(&(Word) {.str = str, .fun = isLongInt});
	str = acceptNext(&(Word) {.str = str, .fun = isCityName});
	return str && str[0] == '\0';
}

static bool isNew(const char *str) {
	str = acceptPrefix(str, "newRoute");
	str = acceptNext(&(Word) {.str = str, .fun = isLongInt});
	str = acceptNext(&(Word) {.str = str, .fun = isCityName});
	str = acceptNext(&(Word) {.str = str, .fun = isCityName});
	return str && str[0] == '\0';

}

static bool isRemRoad(const char *str) {
	str = acceptPrefix(str, "removeRoad");
	str = acceptNext(&(Word) {.str = str, .fun = isCityName});
	str = acceptNext(&(Word) {.str = str, .fun = isCityName});
	return str && str[0] == '\0';
}

static bool isRemRoute(const char *str) {
	str = acceptPrefix(str, "removeRoute");
	str  = acceptNext(&(Word) {.str = str, .fun = isLongInt});
	return str && str[0] == '\0';
}

static Extension *getExtension(char *str) {
	Extension *ans = malloc(sizeof(Extension));
	if (ans) {
		long routeId;
		char *city;
		strtok(str, ";");
		routeId = nextLongInt();
		city = nextWord();
		*ans = (Extension) {.routeId = (unsigned) routeId, .city = city};
		if ((long) ans->routeId == routeId)
			return ans;
	}
	return NULL;
}

static New *getNew(char *str) {
	New *ans = malloc(sizeof(New));
	if (ans) {
		char *city1;
		char *city2;
		strtok(str, ";");
		city1 = nextWord();
		city2 = nextWord();
		*ans = (New) {
			.city1 = city1,
			.city2 = city2
		};
		assert(city1 && city2);
		return ans;
	}
	return NULL;
}

static void doExtension(Extension *ptr) {
	if (ptr) {
		bool success = extendRoute(globalMap, ptr->routeId, ptr->city);
		if (success)
			return;
	}
	writeError();
}

void doNew(New *ptr) {
	if (ptr) {
		bool success;
		success = newRoute(globalMap, ptr->routeId, ptr->city1, ptr->city2);
		if (success)
			return;
	}
	writeError();
}

RemRoad *getRemRoad(char *str) {
	RemRoad *ans = malloc(sizeof(RemRoad));
	if (ans) {
		strtok(str, ";");
		char *city1 = nextWord();
		char *city2 = nextWord();
		*ans = (RemRoad) {
			.city1 = city1,
			.city2 = city2
		};
		return ans;
	}
	return NULL;
}

void doRemRoad(RemRoad *ptr) {
	if (ptr) {
		bool success;
		success = removeRoad(globalMap, ptr->city1, ptr->city2);
		if (success)
			return;
	}
	writeError();
}

void doRemRoute(RemRoute *ptr) {
	if (ptr) {
		bool success;
		success = removeRoute(globalMap, ptr->routeId);
		if (success)
			return;
	}
	writeError();
}

RemRoute *getRemRoute(char *str) {
	RemRoute *ans = malloc(sizeof(RemRoute));
	if (ans) {
		strtok(str, ";");
		long routeId = nextLongInt();
		*ans = (RemRoute) {.routeId = (unsigned) routeId};
		if (ans->routeId == routeId)
			return ans;
		free(ans);
	}
	return NULL;
}

static const char *acceptPrefix(const char *str, const char *prefix) {
	size_t len = strlen(prefix);
	if (str && strncmp(str, prefix, len) == 0)
		return str + len;
	else
		return NULL;
}

