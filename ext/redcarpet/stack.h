#ifndef STACK_H__
#define STACK_H__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct stack {
	void **item;
	size_t size;
	size_t asize;
};

void redcarpet_stack_free(struct stack *);
int redcarpet_stack_grow(struct stack *, size_t);
int redcarpet_stack_init(struct stack *, size_t);

int redcarpet_stack_push(struct stack *, void *);

#ifdef __cplusplus
}
#endif

#endif
