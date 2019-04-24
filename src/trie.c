#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"

#define SQRT_256 16

typedef struct Key {
	const char *str;
	size_t depth;
} Key;

struct Trie {
	City *val;
	Trie *children[SQRT_256];
};

// auxiliary function declarations
size_t keyLength(Key key);
Trie **build(Trie ** trie, City **city, Key key, bool *success);
Trie **getChild(Trie *parent, Key key);

// linked function definitions
Trie **trieInsert(Trie *trie, const char *str, City **city, bool *success) {
	if (trieFind(trie, str))
		return NULL;
	Trie **child = NULL;
	for (size_t depth = 0; str[depth / 2]; ++depth) {
		child = getChild(trie, (Key) {.str = str, .depth = depth});
		if (*child) {
			trie = *child;
		} else {
			return build(&trie, city, (Key) {.str = str, .depth = depth}, success);
		}
	}
	assert(child && !trie->val);
	trie->val = *city;
	return child;
}

City *trieFind(Trie *x, const char *str) {
	Trie *child = x;
	for (size_t depth = 0; str[depth / 2]; ++depth) {
		child = *getChild(x, (Key) {.str = str, .depth = depth});
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


// auxiliary function definitions
size_t keyLength(Key key) {
	size_t ans = strlen(key.str);
	assert(ans <= SIZE_MAX / 2);
	ans *= 2;
	return ans;
}

Trie **build(Trie **trie, City **city, Key key, bool *success) {
	Trie **current = trie;
	for (size_t n = keyLength(key); key.depth < n; ++key.depth) {
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
	return *trie ? getChild(*trie, key) : NULL;
}

Trie **getChild(Trie *parent, Key key) {
	Trie **ans;
	uint8_t childNumber;
	assert(parent);
	childNumber = key.str[key.depth / 2];
	if (key.depth % 2 == 0)
		childNumber /= SQRT_256;
	childNumber %= SQRT_256;
	ans = parent->children + childNumber;
	return ans;
}
