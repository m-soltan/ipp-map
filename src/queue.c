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
	size_t s;
};

// auxiliary function declarations
bool adjust(Heap *heap);
size_t getS(Heap *heap, size_t index);
size_t left(size_t x);
size_t parent(size_t x);
size_t right(size_t x);

// linked function definitions
bool queueEmpty(const Heap *heap) {
	return heap->size == 0;
}

bool queuePush(Heap *heap, City *city, size_t distance) {
	bool adjustSuccess = adjust(heap);
	if (!adjustSuccess)
		return false;
	++heap->size;
	heap->v[heap->size] = (Node) {.city = city, .s = distance};
	for (size_t i = heap->size; i > 1; i = parent(i)) {
		Node temp;
		if (heap->v[parent(i)].s > heap->v[i].s) {
			temp = heap->v[i];
			heap->v[i] = heap->v[parent(i)];
			heap->v[parent(i)] = temp;
		}
	}
	return true;
}

City *queuePeek(Heap *heap) {
	assert(!queueEmpty(heap));
	return heap->v[1].city;
}

City *queuePop(Heap *heap, size_t *distance) {
	Node ans = heap->v[1];
	heap->v[1] = heap->v[heap->size];
	for (size_t i = parent(heap->size); i <= heap->size;) {
		size_t l = getS(heap, left(i)), r = getS(heap, right(i));
		i = (l < r ? left : right)(i);
		if (heap->v[parent(i)].s < heap->v[i].s)
			break;
		heap->v[parent(i)] = heap->v[i];
	}
	--heap->size;
	return ans.city;
}

Heap *queueInit(void) {
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

// auxiliary function definitions
bool adjust(Heap *heap) {
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

size_t getS(Heap *heap, size_t index) {
	if (index && index <= heap->size)
		return heap->v[index].s;
	else
		return SIZE_MAX;
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

void qTest(void) {
	Heap *h = queueInit();
	size_t a, b, c, dist;
	queuePush(h, (City *) 1, 1);
	queuePush(h, (City *) 3, 3);
	queuePush(h, (City *) 2, 2);
	a = (size_t) queuePop(h, &dist);
	b = (size_t) queuePop(h, &dist);
	c = (size_t) queuePop(h, &dist);
	
}
