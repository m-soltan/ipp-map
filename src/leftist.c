#include <stdlib.h>
#include "leftist.h"

typedef struct Node Node;

struct Leftist {
	Node *root;
};

struct Node {
	long s;
	size_t h;
	City *city;
	Node *left, *right;
};

// auxiliary function declarations
Node *merge(Node *x, Node *y);
Node *orderedMerge(Node *x, Node *y);

// auxiliary function definitions

Node *merge(Node *x, Node *y) {
	if (x == NULL || y == NULL)
		return (x == NULL ? y : x);
	if (x->s < y->s)
		return orderedMerge(x, y);
	else
		return orderedMerge(y, x);
}

Node *orderedMerge(Node *x, Node *y) {
	Node *temp;
	x->right = merge(x->right, y);
	if (x->left == NULL) {
		x->left = x->right;
		x->right = NULL;
	} else {
		if (x->left->s < x->right->s) {
			temp = x->left;
			x->left = x->right;
			x->right = temp;
		}
		x->h = x->right->h;
	}
	return x;
}