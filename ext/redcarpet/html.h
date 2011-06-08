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

#ifndef UPSKIRT_HTML_H
#define UPSKIRT_HTML_H

#include "markdown.h"
#include "buffer.h"
#include <stdlib.h>

typedef enum {
	HTML_SKIP_HTML = (1 << 0),
	HTML_SKIP_STYLE = (1 << 1),
	HTML_SKIP_IMAGES = (1 << 2),
	HTML_SKIP_LINKS = (1 << 3),
	HTML_EXPAND_TABS = (1 << 5),
	HTML_SAFELINK = (1 << 7),
	HTML_TOC = (1 << 8),
	HTML_HARD_WRAP = (1 << 9),
	HTML_GITHUB_BLOCKCODE = (1 << 10),
	HTML_USE_XHTML = (1 << 11),
} render_mode;

typedef enum {
	AUTOLINK_URLS = (1 << 0),
	AUTOLINK_EMAILS = (1 << 1),
	AUTOLINK_ALL = AUTOLINK_URLS|AUTOLINK_EMAILS
} autolink_mode;

void
upshtml_escape(struct buf *ob, const char *src, size_t size);

extern void
upshtml_renderer(struct mkd_renderer *renderer, unsigned int render_flags);

extern void
upshtml_toc_renderer(struct mkd_renderer *renderer);

extern void
upshtml_free_renderer(struct mkd_renderer *renderer);

extern void
upshtml_smartypants(struct buf *ob, struct buf *text);

extern void
upshtml_autolink(struct buf *ob, struct buf *text, unsigned int autolink_flags);


#endif

