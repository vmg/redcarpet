/*
 * Copyright (c) 2011, Vicent Marti
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

#ifndef UPSKIRT_AUTOLINK_H
#define UPSKIRT_AUTOLINK_H_H

#include "buffer.h"

typedef enum {
	AUTOLINK_URLS = (1 << 0),
	AUTOLINK_EMAILS = (1 << 1),
	AUTOLINK_ALL = AUTOLINK_URLS|AUTOLINK_EMAILS
} autolink_mode;

extern size_t
ups_autolink__www(size_t *rewind_p, struct buf *link, char *data, size_t offset, size_t size);

extern size_t
ups_autolink__email(size_t *rewind_p, struct buf *link, char *data, size_t offset, size_t size);

extern size_t
ups_autolink__url(size_t *rewind_p, struct buf *link, char *data, size_t offset, size_t size);

#endif

/* vim: set filetype=c: */
