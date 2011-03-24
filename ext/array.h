/* array.h - automatic dynamic array for pointers */

/*
 * Copyright (c) 2008, Natacha Porté
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef LITHIUM_ARRAY_H
#define LITHIUM_ARRAY_H

#include <stdlib.h>


/********************
 * TYPE DEFINITIONS *
 ********************/

/* struct array • generic linear array */
struct array {
	void*	base;
	int	size;
	int	asize;
	size_t	unit; };


/* struct parray • array of pointers */
struct parray {
	void **	item;
	int	size;
	int	asize; };


/* array_cmp_fn • comparison functions for sorted arrays */
typedef int (*array_cmp_fn)(void *key, void *array_entry);



/***************************
 * GENERIC ARRAY FUNCTIONS *
 ***************************/

/* arr_adjust • shrink the allocated memory to fit exactly the needs */
int
arr_adjust(struct array *);

/* arr_free • frees the structure contents (buf NOT the struct itself) */
void
arr_free(struct array *);

/* arr_grow • increases the array size to fit the given number of elements */
int
arr_grow(struct array *, int);

/* arr_init • initialization of the contents of the struct */
void
arr_init(struct array *, size_t);

/* arr_insert • inserting elements nb before the nth one */
int
arr_insert(struct array *, int nb, int n);

/* arr_item • returns a pointer to the n-th element */
void *
arr_item(struct array *, int);

/* arr_newitem • returns the index of a new element appended to the array */
int
arr_newitem(struct array *);

/* arr_remove • removes the n-th elements of the array */
void
arr_remove(struct array *, int);

/* arr_sorted_find • O(log n) search in a sorted array, returning entry */
/* equivalent to bsearch(key, arr->base, arr->size, arr->unit, cmp) */
void *
arr_sorted_find(struct array *, void *key, array_cmp_fn cmp);

/* arr_sorted_find_i • O(log n) search in a sorted array,
 *      returning index of the smallest element larger than the key */
int
arr_sorted_find_i(struct array *, void *key, array_cmp_fn cmp);


/***************************
 * POINTER ARRAY FUNCTIONS *
 ***************************/

/* parr_adjust • shrinks the allocated memory to fit exactly the needs */
int
parr_adjust(struct parray *);

/* parr_free • frees the structure contents (buf NOT the struct itself) */
void
parr_free(struct parray *);

/* parr_grow • increases the array size to fit the given number of elements */
int
parr_grow(struct parray *, int);

/* parr_init • initialization of the struct (which is equivalent to zero) */
void
parr_init(struct parray *);

/* parr_insert • inserting nb elements before the nth one */
int
parr_insert(struct parray *, int nb, int n);

/* parr_pop • pops the last item of the array and returns it */
void *
parr_pop(struct parray *);

/* parr_push • pushes a pointer at the end of the array (= append) */
int
parr_push(struct parray *, void *);

/* parr_remove • removes the n-th element of the array and returns it */
void *
parr_remove(struct parray *, int);

/* parr_sorted_find • O(log n) search in a sorted array, returning entry */
void *
parr_sorted_find(struct parray *, void *key, array_cmp_fn cmp);

/* parr_sorted_find_i • O(log n) search in a sorted array,
 *      returning index of the smallest element larger than the key */
int
parr_sorted_find_i(struct parray *, void *key, array_cmp_fn cmp);

/* parr_top • returns the top the stack (i.e. the last element of the array) */
void *
parr_top(struct parray *);


#endif /* ndef LITHIUM_ARRAY_H */

/* vim: set filetype=c: */
