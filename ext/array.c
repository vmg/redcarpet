/* array.c - automatic dynamic array for pointers */

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

#include "array.h"

#include <string.h>


/***************************
 * STATIC HELPER FUNCTIONS *
 ***************************/

/* arr_realloc • realloc memory of a struct array */
static int
arr_realloc(struct array* arr, int neosz) {
	void* neo;
	neo = realloc(arr->base, neosz * arr->unit);
	if (neo == 0) return 0;
	arr->base = neo;
	arr->asize = neosz;
	if (arr->size > neosz) arr->size = neosz;
	return 1; }


/* parr_realloc • realloc memory of a struct parray */
static int
parr_realloc(struct parray* arr, int neosz) {
	void* neo;
	neo = realloc(arr->item, neosz * sizeof (void*));
	if (neo == 0) return 0;
	arr->item = neo;
	arr->asize = neosz;
	if (arr->size > neosz) arr->size = neosz;
	return 1; }



/***************************
 * GENERIC ARRAY FUNCTIONS *
 ***************************/

/* arr_adjust • shrink the allocated memory to fit exactly the needs */
int
arr_adjust(struct array *arr) {
	return arr_realloc(arr, arr->size); }


/* arr_free • frees the structure contents (buf NOT the struct itself) */
void
arr_free(struct array *arr) {
	if (!arr) return;
	free(arr->base);
	arr->base = 0;
	arr->size = arr->asize = 0; }


/* arr_grow • increases the array size to fit the given number of elements */
int
arr_grow(struct array *arr, int need) {
	if (arr->asize >= need) return 1;
	else return arr_realloc(arr, need); }


/* arr_init • initialization of the contents of the struct */
void
arr_init(struct array *arr, size_t unit) {
	arr->base = 0;
	arr->size = arr->asize = 0;
	arr->unit = unit; }


/* arr_insert • inserting nb elements before the nth one */
int
arr_insert(struct array *arr, int nb, int n) {
	char *src, *dst;
	size_t len;
	if (!arr || nb <= 0 || n < 0
	|| !arr_grow(arr, arr->size + nb))
		return 0;
	if (n < arr->size) {
		src = arr->base;
		src += n * arr->unit;
		dst = src + nb * arr->unit;
		len = (arr->size - n) * arr->unit;
		memmove(dst, src, len); }
	arr->size += nb;
	return 1; }


/* arr_item • returns a pointer to the n-th element */
void *
arr_item(struct array *arr, int no) {
	char *ptr;
	if (!arr || no < 0 || no >= arr->size) return 0;
	ptr = arr->base;
	ptr += no * arr->unit;
	return ptr; }


/* arr_newitem • returns the index of a new element appended to the array */
int
arr_newitem(struct array *arr) {
	if (!arr_grow(arr, arr->size + 1)) return -1;
	arr->size += 1;
	return arr->size - 1; }


/* arr_remove • removes the n-th elements of the array */
void
arr_remove(struct array *arr, int idx) {
	if (!arr || idx < 0 || idx >= arr->size) return;
	arr->size -= 1;
	if (idx < arr->size) {
		char *dst = arr->base;
		char *src;
		dst += idx * arr->unit;
		src = dst + arr->unit;
		memmove(dst, src, (arr->size - idx) * arr->unit); } }


/* arr_sorted_find • O(log n) search in a sorted array, returning entry */
void *
arr_sorted_find(struct array *arr, void *key, array_cmp_fn cmp) {
	int mi, ma, cu, ret;
	char *ptr = arr->base;
	mi = -1;
	ma = arr->size;
	while (mi < ma - 1) {
		cu = mi + (ma - mi) / 2;
		ret = cmp(key, ptr + cu * arr->unit);
		if (ret == 0) return ptr + cu * arr->unit;
		else if (ret < 0) ma = cu;
		else /* if (ret > 0) */ mi = cu; }
	return 0; }


/* arr_sorted_find_i • O(log n) search in a sorted array,
 *      returning index of the smallest element larger than the key */
int
arr_sorted_find_i(struct array *arr, void *key, array_cmp_fn cmp) {
	int mi, ma, cu, ret;
	char *ptr = arr->base;
	mi = -1;
	ma = arr->size;
	while (mi < ma - 1) {
		cu = mi + (ma - mi) / 2;
		ret = cmp(key, ptr + cu * arr->unit);
		if (ret == 0) {
			while (cu < arr->size && ret == 0) {
				cu += 1;
				ret = cmp(key, ptr + cu * arr->unit); }
			return cu; }
		else if (ret < 0) ma = cu;
		else /* if (ret > 0) */ mi = cu; }
	return ma; }



/***************************
 * POINTER ARRAY FUNCTIONS *
 ***************************/

/* parr_adjust • shrinks the allocated memory to fit exactly the needs */
int
parr_adjust(struct parray* arr) {
	return parr_realloc (arr, arr->size); }


/* parr_free • frees the structure contents (buf NOT the struct itself) */
void
parr_free(struct parray *arr) {
	if (!arr) return;
	free (arr->item);
	arr->item = 0;
	arr->size = 0;
	arr->asize = 0; }


/* parr_grow • increases the array size to fit the given number of elements */
int
parr_grow(struct parray *arr, int need) {
	if (arr->asize >= need) return 1;
	else return parr_realloc (arr, need); }


/* parr_init • initialization of the struct (which is equivalent to zero) */
void
parr_init(struct parray *arr) {
	arr->item = 0;
	arr->size = 0;
	arr->asize = 0; }


/* parr_insert • inserting nb elements before the nth one */
int
parr_insert(struct parray *parr, int nb, int n) {
	char *src, *dst;
	size_t len, i;
	if (!parr || nb <= 0 || n < 0
	|| !parr_grow(parr, parr->size + nb))
		return 0;
	if (n < parr->size) {
		src = (void *)parr->item;
		src += n * sizeof (void *);
		dst = src + nb * sizeof (void *);
		len = (parr->size - n) * sizeof (void *);
		memmove(dst, src, len);
		for (i = 0; i < nb; ++i)
			parr->item[n + i] = 0; }
	parr->size += nb;
	return 1; }


/* parr_pop • pops the last item of the array and returns it */
void *
parr_pop(struct parray *arr) {
	if (arr->size <= 0) return 0;
	arr->size -= 1;
	return arr->item[arr->size]; }


/* parr_push • pushes a pointer at the end of the array (= append) */
int
parr_push(struct parray *arr, void *i) {
	if (!parr_grow(arr, arr->size + 1)) return 0;
	arr->item[arr->size] = i;
	arr->size += 1;
	return 1; }


/* parr_remove • removes the n-th element of the array and returns it */
void *
parr_remove(struct parray *arr, int idx) {
	void* ret;
	int i;
	if (!arr || idx < 0 || idx >= arr->size) return 0;
	ret = arr->item[idx];
	for (i = idx+1; i < arr->size; ++i)
		arr->item[i - 1] = arr->item[i];
	arr->size -= 1;
	return ret; }


/* parr_sorted_find • O(log n) search in a sorted array, returning entry */
void *
parr_sorted_find(struct parray *arr, void *key, array_cmp_fn cmp) {
	int mi, ma, cu, ret;
	mi = -1;
	ma = arr->size;
	while (mi < ma - 1) {
		cu = mi + (ma - mi) / 2;
		ret = cmp(key, arr->item[cu]);
		if (ret == 0) return arr->item[cu];
		else if (ret < 0) ma = cu;
		else /* if (ret > 0) */ mi = cu; }
	return 0; }


/* parr_sorted_find_i • O(log n) search in a sorted array,
 *      returning index of the smallest element larger than the key */
int
parr_sorted_find_i(struct parray *arr, void *key, array_cmp_fn cmp) {
	int mi, ma, cu, ret;
	mi = -1;
	ma = arr->size;
	while (mi < ma - 1) {
		cu = mi + (ma - mi) / 2;
		ret = cmp(key, arr->item[cu]);
		if (ret == 0) {
			while (cu < arr->size && ret == 0) {
				cu += 1;
				ret = cmp(key, arr->item[cu]); }
			return cu; }
		else if (ret < 0) ma = cu;
		else /* if (ret > 0) */ mi = cu; }
	return ma; }


/* parr_top • returns the top the stack (i.e. the last element of the array) */
void *
parr_top(struct parray *arr) {
	if (arr == 0 || arr->size <= 0) return 0;
	else return arr->item[arr->size - 1]; }

/* vim: set filetype=c: */
