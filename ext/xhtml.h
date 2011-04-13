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

#ifndef UPSKIRT_XHTML_H
#define UPSKIRT_XHTML_H

typedef enum {
	XHTML_SKIP_HTML = (1 << 0),
	XHTML_SKIP_STYLE = (1 << 1),
	XHTML_SKIP_IMAGES = (1 << 2),
	XHTML_SKIP_LINKS = (1 << 3),
	XHTML_SMARTYPANTS = (1 << 4),
	XHTML_EXPAND_TABS = (1 << 5),
	XHTML_SAFELINK = (1 << 7),
	XHTML_TOC = (1 << 8),
	XHTML_STRIKETHROUGH = (1 << 10),
} render_mode;

void
init_xhtml_renderer(struct mkd_renderer *renderer, unsigned int render_flags);

void
init_toc_renderer(struct mkd_renderer *renderer);

void
free_renderer(struct mkd_renderer *renderer);

#endif

