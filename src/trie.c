#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"

#define SQRT_256 16

typedef struct Key Key;
typedef struct TrieStore TrieStore;

struct Key {
	const char *str;
	size_t depth, length;
};

struct Trie {
	City *val;
	Trie *children[SQRT_256];
};

struct TrieStore {
	size_t length;
	Trie **arr;
};

static bool hasNext(Key key);
static bool storePrepare(size_t count);
static void add(Trie *trie, const char *str, City *city);
static void build(Trie **trie, Key key, City *city);
static void storeDrop();
static City *getVal(Trie *trie);
static Key makeKey(const char *str);
static Trie *find(Trie *trie, Key key);
static Trie *storeTake();

static Trie **getChild(Trie *parent, Key key);

static TrieStore trieStore = (TrieStore) {.length = 0, .arr = NULL};

bool trieInsert(Trie *trie, const char *str, City *city) {
	bool success;
	size_t length = strlen(str);
	success = storePrepare(2 * length);
	if (success) {
		add(trie, str, city);
		storeDrop();
		return true;
	}
	return false;
}

City *trieFind(Trie *trie, const char *str) {
	return getVal(find(trie, makeKey(str)));
}

Trie *trieInit() {
	bool success = storePrepare(1);
	if (success) {
		Trie *ans = storeTake();
		storeDrop();
		return ans;
	} else {
		return NULL;
	}
}

void trieDestroy(Trie **pTrie) {
	Trie *t = *pTrie;
	if (!t)
		return;
	for (size_t i = 0; i < SQRT_256; ++i)
		trieDestroy(&t->children[i]);
	free(t);
	*pTrie = NULL;
}

bool trieAddFromList(Trie *trie, NameList list, City *const *cities) {
	bool success;
	size_t totalLength = 0;
	for (size_t i = 0; i < list.length; ++i)
		totalLength += strlen(list.v[i]);
	success = storePrepare(2 * totalLength);
	if(success) {
		for (size_t i = 0; i < list.length; ++i)
			add(trie, list.v[i], cities[i]);
		storeDrop();
		return true;
	}
	return false;
}

static Trie **getChild(Trie *parent, Key key) {
	Trie **ans;
	uint8_t childNumber;
	assert(parent);
	assert(key.depth / 2 < key.length);
	childNumber = (uint8_t) key.str[key.depth / 2];
	if (key.depth % 2 == 0)
		childNumber /= SQRT_256;
	childNumber %= SQRT_256;
	ans = &parent->children[childNumber];
	return ans;
}

static Trie *find(Trie *trie, Key key) {
	Trie *child = trie;
	for (; hasNext(key); ++key.depth) {
		child = *getChild(trie, key);
		if (child)
			trie = child;
		else
			return NULL;
	}
	return child;
}

static Key makeKey(const char *str) {
	return (Key) {
		.str = str,
		.depth = 0,
		.length = strlen(str),
	};
}

static bool hasNext(Key key) {
	return key.str[key.depth / 2] != '\0';
}

static Trie *storeTake() {
	assert(0 < trieStore.length);
	Trie *ans = trieStore.arr[trieStore.length - 1];
	--trieStore.length;
	return ans;
}

static bool storePrepare(size_t count) {
	assert(trieStore.length == 0);
	trieStore.arr = calloc(count, sizeof(Trie *));
	if (trieStore.arr) {
		for (; trieStore.length < count; ++trieStore.length) {
			trieStore.arr[trieStore.length] = calloc(1, sizeof(Trie));
			if (trieStore.arr[trieStore.length] == NULL)
				break;
		}
		if (trieStore.length == count)
			return true;
		storeDrop();
	}
	return false;
}

static void storeDrop() {
	for (size_t i = 0; i < trieStore.length; ++i) {
		free(trieStore.arr[i]);
	}
	free(trieStore.arr);
	trieStore = (TrieStore) {.arr = NULL, .length = 0};
}

static void add(Trie *trie, const char *str, City *city) {
	Key key = makeKey(str);
	Trie **child = NULL, **parent = &trie;
	for (; hasNext(key); ++key.depth) {
		child = getChild(*parent, key);
		if (*child)
			parent = child;
		else
			return build(parent, key, city);
	}
}

void build(Trie **trie, Key key, City *city) {
	Trie **current = trie;
	for (const size_t n = key.length * 2; key.depth < n; ++key.depth) {
		current = getChild(*current, key);
		*current = storeTake();
	}
	(*current)->val = city;
}

City *getVal(Trie *trie) {
	if (trie)
		return trie->val;
	else
		return NULL;
}
