#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"

#define SQRT_256 16

typedef struct Key Key;

struct Key {
	const char *str;
	size_t depth, length;
};

struct Trie {
	City *val;
	Trie *children[SQRT_256];
};

Trie **build(Trie ** trie, City **city, Key key, bool *success);
Trie **getChild(Trie *parent, Key key);


// linked function definitions
Trie **trieInsert(Trie *trie, const char *str, City **city, bool *success) {
	Trie **child = NULL;
	if (trieFind(trie, str)) {
		*success = false;
		return NULL;
	}
	Key key = (Key) {.str = str, .length = strlen(str)};
	for (key.depth = 0; str[key.depth / 2]; ++key.depth) {
		child = getChild(trie, key);
		if (*child) {
			trie = *child;
		} else {
			return build(&trie, city, key, success);
		}
	}
	assert(child && !trie->val);
	trie->val = *city;
	*success = true;
	return child;
}

City *trieFind(Trie *x, const char *str) {
	Key key = (Key) {.str = str, .length = strlen(str)};
	Trie *child = x;
	for (; str[key.depth / 2]; ++key.depth) {
		child = *getChild(x, key);
		if (child)
			x = child;
		else
			return NULL;
	}
	return child->val;
}

Trie *trieInit() {
	Trie *ans = malloc(sizeof(Trie));
	if (ans) {
		ans->val = NULL;
		memset(ans->children, 0, SQRT_256 * sizeof(Trie *));
	}
	return ans;
}

void trieDestroy(Trie **x) {
	Trie *t = *x;
	if (!t)
		return;
	for (size_t i = 0; i < SQRT_256; ++i)
		trieDestroy(&t->children[i]);
	free(t);
	*x = NULL;
}


Trie **build(Trie **const trie, City **city, Key key, bool *success) {
	Trie **ans = getChild(*trie, key), **current = trie;
	for (const size_t n = key.length * 2; key.depth < n; ++key.depth) {
		current = getChild(*current, key);
		*current = trieInit();
		if (!*current) {
			trieDestroy(getChild(*trie, key));
			free(*city);
			*success = false;
			return NULL;
		}
	}
	(*current)->val = *city;
	*success = true;
	return ans;
}

Trie **getChild(Trie *parent, Key key) {
	Trie **ans;
	uint8_t childNumber;
	if (parent == NULL)
		return NULL;
	assert(key.depth / 2 < key.length);
	childNumber = (uint8_t) key.str[key.depth / 2];
	if (key.depth % 2 == 0)
		childNumber /= SQRT_256;
	childNumber %= SQRT_256;
	ans = parent->children + childNumber;
	return ans;
}
