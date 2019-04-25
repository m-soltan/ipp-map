#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "trunk.h"

//typedef struct Node Node;

struct List {
	Trunk *v;
	List *tail;
};

Trunk *listPop(List **l) {
	List *head;
	Trunk *ans;
	assert(*l != NULL);
	head = *l;
	ans = head->v;
	*l = head->tail;
	free(head);
	return ans;
}

bool listPush(List **l, Trunk *value) {
	List *head = malloc(sizeof(List));
	if (head == NULL)
		return false;
	head->v = value;
	head->tail = *l;
	*l = head;
	return true;
}
//struct Trunk {
//	Node *first, *last;
//};

//struct Node {
//	Road *road;
//	Node *prev, *next;
//};
