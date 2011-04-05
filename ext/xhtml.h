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
	RENDER_SKIP_HTML = (1 << 0),
	RENDER_SKIP_STYLE = (1 << 1),
	RENDER_SKIP_IMAGES = (1 << 2),
	RENDER_SKIP_LINKS = (1 << 3),
	RENDER_SMARTYPANTS = (1 << 4),
	RENDER_EXPAND_TABS = (1 << 5),
	RENDER_AUTOLINK = (1 << 6),
	RENDER_SAFELINK = (1 << 7),
	RENDER_TOC = (1 << 8),
} render_mode;

void
init_xhtml_renderer(struct mkd_renderer *renderer,
		unsigned int render_flags,
		unsigned int parser_flags, int recursion_depth);

void
init_toc_renderer(struct mkd_renderer *renderer, int recursion_depth);

void
free_renderer(struct mkd_renderer *renderer);

#endif

