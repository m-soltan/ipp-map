#include "trunk.h"

typedef struct Node Node;

struct Trunk {
	Node *first, *last;
};

struct Node {
	Road *road;
	Node *prev, *next;
};
