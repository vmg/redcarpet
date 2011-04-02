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

#include "markdown.h"

#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

static void
toc_header(struct buf *ob, struct buf *text, int level, int header_id, struct mkd_renderopt *options)
{
	int current_level = (int)options->opaque.data;

	if (level > current_level) {
		if (level > 1)
			BUFPUTSL(ob, "<li>");
		BUFPUTSL(ob, "<ul>\n");
	}
	
	if (level < current_level) {
		BUFPUTSL(ob, "</ul>");
		if (current_level > 1)
			BUFPUTSL(ob, "</li>\n");
	}

	options->opaque.data = level;

	bufprintf(ob, "<li><a href=\"#toc_%d\">", header_id);
	if (text)
		bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</a></li>\n");
}

static void
toc_finalize(struct buf *ob, struct mkd_renderopt *options)
{
	int current_level = (int)options->opaque.data;

	while (current_level > 1) {
		BUFPUTSL(ob, "</ul></li>\n");
		current_level--;
	}

	if (current_level)
		BUFPUTSL(ob, "</ul>\n");
}

void
init_toc_renderer(struct mkd_renderer *renderer, int recursion_depth)
{
	static const struct mkd_renderer toc_render = {
		NULL,
		NULL,
		NULL,
		toc_header,
		NULL,
		NULL,
		NULL,
		NULL,

		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,

		NULL,
		NULL,
		toc_finalize,

		NULL,

		{ 0, 0 },
		{ 0, 0 },
	};

	memcpy(renderer, &toc_render, sizeof(struct mkd_renderer));

	renderer->parser_options.recursion_depth = recursion_depth;
	renderer->render_options.flags = RENDER_TOC;
	renderer->render_options.opaque.data = 0;
}

