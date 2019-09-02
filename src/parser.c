#include <ctype.h>
#include <stdio.h>
#include "map.h"
#include "parser.h"

#define COMMENT_SYMBOL '#'
#define INIT_BUFFER_SIZE 2

typedef struct AddRoad AddRoad;
typedef struct Creation Creation;
typedef struct Extension Extension;
typedef struct Description Description;
typedef struct NewRoute NewRoute;
typedef struct RemRoad RemRoad;
typedef struct RemRoute RemRoute;
typedef struct Repair Repair;
typedef struct Word Word;

/// a struct storing information about the addRoad parser command
struct AddRoad {
	/// a city the road is connected to
	char *city1;
	/// a city the road is connected to
	char *city2;
	/// the year the road was built
	int builtYear;
	/// the length of the road to be added
	unsigned length;
};

/** @brief A struct storing information about a parser command.
 * Describes the command that creates a route from path description.
 */
struct Creation {
	/// id of the route to be created
	unsigned routeId;
	/// explicit struct padding
	unsigned pad;
	/// list of the names of the cities the route will use
	const char **cityNames;
	/// list of the lengths of the roads used by the route
	unsigned *roadLengths;
	/// list of the years the roads were last repaired or built
	int *builtYears;
	/// current length of the lists
	size_t length;
	/// total space available to each list
	size_t lengthMax;

};

/// A struct storing information about the getRouteDescription parser command.
struct Description {
	/// id of the examined route
	unsigned routeId;
};

/// A struct storing information about the extendRoute parser command.
struct Extension {
	/// id of the route to be extended
	unsigned routeId;
	/// explicit struct padding
	unsigned pad;
	/// the city the extension will end in
	char *city;
};

/// A struct storing information about the newRoute parser command.
struct NewRoute {
	/// the id of the route to be added
	unsigned routeId;
	/// explicit struct padding
	unsigned pad;
	/// the starting city for the route
	char *city1;
	/// the final city of the route
	char *city2;
};

/// A struct storing information about the removeRoad parser command.
 struct RemRoad {
	 /// a city that the road due for removal is connected to
	char *city1;
	/// a city that the road due for removal is connected to
	char *city2;
};

/// A struct storing information about the removeRoute parser command.
struct RemRoute {
	/// the id of the route to be removed
	unsigned routeId;
	/// explicit struct padding
	unsigned pad;
};

/** A struct storing information about the repairRoad parser command.
 *
 */
struct Repair {
	/// a city connected to the road due for repair
	char *city1;
	/// a city connected to the road due for repair
	char *city2;
	/// the year of the repair to be recorded
	int repairYear;
	/// explicit struct padding
	int pad;
};

/** A struct used to simplify parsing.
 * Stores a string and the next function to be used when parsing
 * that string.
 */
struct Word {
	/// the remainder of the parser command
	const char *str;
	/// the function used to parse the next part of the string
	size_t (*fun)(const char *);
};

//! @cond
static bool charIsLetter(char c);
static bool isAddition(const char *str);
static bool isComment(const char *str);
static bool isCreation(const char *str);
static bool isDescription(const char *str);
static bool isExtension(const char *str);
static bool isNewRoute(const char *str);
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
static void creationDestroy(Creation **pCreation);
static void doAddition(AddRoad *ptr);
static void doCreation(Creation *ptr);
static void doDescription(Description *ptr);
static void doExtension(Extension *ptr);
static void doNewRoute(NewRoute *ptr);
static void doRemRoad(RemRoad *ptr);
static void doRemRoute(RemRoute *ptr);
static void doRepair(Repair *ptr);
static AddRoad *getAddition(char *str);
static Creation *getCreation(char *str);
static Description *getDescription(char *str);
static Extension *getExtension(char *str);
static NewRoute *getNewRoute(char *str);
static RemRoad *getRemRoad(char *str);
static RemRoute *getRemRoute(char *str);
static Repair *getRepair(char *str);
//! @endcond

/// the main map structure
static Map *globalMap = NULL;
/// the number of the line, used for error messages
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
		assert(c != EOF);
		if (c == '\n')
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
		;
	// getRouteDescription
	else if (isDescription(line))
		doDescription(getDescription(line));
	// extendRoute
	else if (isExtension(line))
		doExtension(getExtension(line));
	// newRoute
	else if (isNewRoute(line))
		doNewRoute(getNewRoute(line));
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

int runParser(void) {
	bool success_init;
	success_init = setMap();
	if (!success_init)
		return OUT_OF_MEMORY;
	while (true) {
		int c = fgetc(stdin);
		if (c == EOF) {
			deleteMap(globalMap);
			return 0;
		} else {
			ungetc(c, stdin);
		}
		char *line = getLine();
		if (line == NULL) {
			deleteMap(globalMap);
			return OUT_OF_MEMORY;
		}
		parserRead(line);
	}
}

//! @cond
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
		char c;
		str = acceptNext(&(Word) {.str = str, .fun = isCityName});
		if (str == NULL)
			return false;
		c = str[0];
		if (c != ';')
			return c == '\0' && i > 0;
		for (int j = 0; j < 2; ++j)
			str = acceptNext(&(Word) {.str = str, .fun = isLongInt});
	}
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

static void doAddition(AddRoad *ptr) {
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

static AddRoad *getAddition(char *str) {
	AddRoad *ans = malloc(sizeof(AddRoad));
	if (ans) {
		strtok(str, ";");
		char *city1 = nextWord();
		char *city2 = nextWord();
		long length = nextLongInt();
		long builtYear = nextLongInt();
		*ans = (AddRoad) {
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
		return str[1] == '\0';
	for (size_t i = 0; true; ++i) {
		if (str[i] == ';' || str[i] == '\0')
			return i;
		if (i == 0 && str[i] == '-' && str[i + 1] != '\0')
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
		free(ptr);
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
	Creation *ans = calloc(1, sizeof(Creation));
	if (ans) {
		long routeId = strtol(strtok(str, ";"), NULL, 10);
		ans->routeId = (unsigned) routeId;
		if (ans->routeId == routeId) {
			for (bool stay = true; stay;) {
				int pushResult;
				char *cityName = nextWord();
				long roadLength = nextLongInt();
				long builtYear = nextLongInt();
				if (roadLength == 0 || builtYear == 0)
					stay = false;
				pushResult = push(ans, cityName, roadLength, builtYear);
				if (pushResult != 0)
					return NULL;
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
	if (!lengthSuccess || !yearSuccess) {
		creationDestroy(&c);
		return INVALID_ARG;
	}
	++c->length;
	return 0;
}

static void creationDestroy(Creation **pCreation) {
	Creation *creation = *pCreation;
	*pCreation = NULL;
	void *arr[] = {
			creation->cityNames,
			creation->builtYears,
			creation->roadLengths
	};
	for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); ++i)
		if (arr[i])
			free(arr[i]);
	free(creation);
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
				ptr->length
		);
		creationDestroy(&ptr);
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

static bool isNewRoute(const char *str) {
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
		free(ans);
	}
	return NULL;
}

static NewRoute *getNewRoute(char *str) {
	NewRoute *ans = calloc(1, sizeof(NewRoute));
	if (ans) {
		long routeId;
		strtok(str, ";");
		routeId = nextLongInt();
		ans->routeId = (unsigned) routeId;
		ans->city1 = nextWord();
		ans->city2 = nextWord();
		assert(ans->city1 && ans->city2);
		if (ans->routeId == routeId)
			return ans;
		else
			free(ans);
	}
	return NULL;
}

static void doExtension(Extension *ptr) {
	if (ptr) {
		bool success = extendRoute(globalMap, ptr->routeId, ptr->city);
		free(ptr);
		if (success)
			return;
	}
	writeError();
}

static void doNewRoute(NewRoute *ptr) {
	if (ptr) {
		bool success;
		success = newRoute(globalMap, ptr->routeId, ptr->city1, ptr->city2);
		free(ptr);
		if (success)
			return;
	}
	writeError();
}

static RemRoad *getRemRoad(char *str) {
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

static void doRemRoad(RemRoad *ptr) {
	if (ptr) {
		bool success;
		success = removeRoad(globalMap, ptr->city1, ptr->city2);
		free(ptr);
		if (success)
			return;
	}
	writeError();
}

static void doRemRoute(RemRoute *ptr) {
	if (ptr) {
		bool success;
		success = removeRoute(globalMap, ptr->routeId);
		free(ptr);
		if (success)
			return;
	}
	writeError();
}

static RemRoute *getRemRoute(char *str) {
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
//! @endcond
