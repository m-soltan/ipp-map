#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "trunk.h"


struct Trunk {
	size_t number;
	City **cities;
};

//struct Node {
//	Road *road;
//	Node *prev, *next;
//};
