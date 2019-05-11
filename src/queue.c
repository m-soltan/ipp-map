#include <assert.h>
#include <stdlib.h>
#include "queue.h"

typedef struct Node Node;

struct Heap {
	Node *v;
	size_t size, sizeMax;
};


struct Node {
	City *city;
	Road *last;
	int minYear;
	char pad[sizeof(size_t) - sizeof(int)];
	size_t s;
};

// auxiliary function declarations
bool queueAdjust(Heap *heap);
bool greater(const Heap *heap, size_t lhs, size_t rhs);

size_t left(size_t x);
size_t parent(size_t x);
size_t right(size_t x);

// linked function definitions
bool queueEmpty(const Heap *heap) {
	return heap->size == 0;
}

bool queuePush(Heap *heap, Road *road, City *city, size_t distance, int minYear) {
	bool adjustSuccess = queueAdjust(heap);
	Node *arr = heap->v;
	if (!adjustSuccess)
		return false;
	++heap->size;
	arr[heap->size] = (Node) {.city = city, .s = distance, .minYear = minYear, .last = road};
	for (size_t i = heap->size; true; i = parent(i)) {
		if (greater(heap, i, parent(i)))
			break;
		Node temp = heap->v[i];
		heap->v[i] = heap->v[parent(i)];
		heap->v[parent(i)] = temp;
	}
	assert(heap->v[1].city);
	return true;
}

Road *queuePop(Heap *heap, size_t *distance, int *minYear, City **pCity) {
	Node *arr = heap->v, ans;
	size_t temp;
	ans = arr[1];
	temp = heap->size;
	for (size_t i = 1, next; true; i = next) {
		if (greater(heap, left(i), temp) && greater(heap, right(i), temp)) {
			arr[i] = arr[heap->size];
			break;
		}
		next = (greater(heap, left(i), right(i)) ? right : left)(i);
		arr[i] = arr[next];
	}
	assert(heap->size == 0 || heap->v[1].city != NULL);
	--heap->size;
	*distance = ans.s;
	*minYear = ans.minYear;
	*pCity = ans.city;
	return ans.last;
}

Heap *queueInit() {
	Heap *ans = malloc(sizeof(Heap));
	if (ans) {
		*ans = (Heap) {.size = 0, .sizeMax = 8};
		ans->v = malloc(ans->sizeMax * sizeof(Node));
		if (ans->v)
			return ans;
		free(ans);
	}
	return NULL;
}

void queueDestroy(Heap **pHeap) {
	free((*pHeap)->v);
	free(*pHeap);
	*pHeap = NULL;
}

// auxiliary function definitions
bool queueAdjust(Heap *heap) {
	assert(heap && heap->sizeMax > heap->size);
	if (heap->size < heap->sizeMax)
		return true;
	Node *tmp = realloc(heap->v, 2 * heap->sizeMax * sizeof(Node));
	if (tmp) {
		heap->v = tmp;
		heap->sizeMax *= 2;
		return true;
	} else {
		return false;
	}
}

bool greater(const Heap *heap, size_t lhs, size_t rhs) {
	if (lhs > heap->size || rhs > heap->size || rhs < 1 || lhs < 1)
		return lhs > heap->size || rhs < 1;
	Node *lNode = &heap->v[lhs], *rNode = &heap->v[rhs];
	if (lNode->s == rNode->s)
		return (lNode->minYear < rNode->minYear);
	else
		return lNode->s > rNode->s;
}

size_t left(size_t x) {
	return x * 2;
}

size_t parent(size_t x) {
	return x / 2;
}

size_t right(size_t x) {
	return 1 + x * 2;
}
