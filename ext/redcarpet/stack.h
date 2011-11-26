#ifndef STACK_H__
#define STACK_H__

#include <stdlib.h>

struct stack {
	void **item;
	size_t size;
	size_t asize;
};

void stack_free(struct stack *);
int stack_grow(struct stack *, size_t);
int stack_init(struct stack *, size_t);

int stack_push(struct stack *, void *);

void *stack_pop(struct stack *);
void *stack_top(struct stack *);

#endif
