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
#include "buffer.h"
#include "html.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static inline int
is_closing_a(const char *tag, size_t size)
{
	size_t i;

	if (tag[0] != '<' || size < STRLEN("</a>") || tag[1] != '/')
		return 0;

	i = 2;

	while (i < size && isspace(tag[i]))
		i++;

	if (i == size || tag[i] != 'a')
		return 0;

	i++;

	while (i < size && isspace(tag[i]))
		i++;

	if (i == size || tag[i] != '>')
		return 0;

	return i;
}

static size_t
skip_tags(struct buf *ob, const char *text, size_t size)
{
	size_t i = 0;

	while (i < size && text[i] != '>')
		i++;

	if (size > 3 && text[1] == 'a' && isspace(text[2])) {
		while (i < size) {
			size_t tag_len = is_closing_a(text + i, size - i);
			if (tag_len) {
				i += tag_len;
				break;
			}
			i++;
		}
	}

	bufput(ob, text, i + 1);
	return i + 1;
}

void
upshtml_autolink(struct buf *ob, struct buf *text, unsigned int flags)
{
	size_t i, end;
	struct buf *link = bufnew(16);
	const char *active_chars;

	if (!text || text->size == 0)
		return;

	switch (flags) {
		case AUTOLINK_EMAILS:
			active_chars = "<@";
			break;

		case AUTOLINK_URLS:
			active_chars = "<w:";

		case AUTOLINK_ALL:
			active_chars = "<@w:";
			break;

		default:
			return;
	}

	bufgrow(ob, text->size);

	i = end = 0;

	while (i < text->size) {
		size_t rewind;

		while (end < text->size && strchr(active_chars, text->data[end]) == NULL)
			end++;

		bufput(ob, text->data + i, end - i);

		if (end >= text->size)
			break;

		i = end;
		link->size = 0;

		switch (text->data[i]) {
		case '@':
			end = ups_autolink__email(&rewind, link, text->data + i, i, text->size - i);
			if (end > 0) {
				ob->size -= rewind;
				BUFPUTSL(ob, "<a href=\"mailto:");
				bufput(ob, link->data, link->size);
				BUFPUTSL(ob, "\">");
				upshtml_escape(ob, link->data, link->size);
				BUFPUTSL(ob, "</a>");
			}
			break;

		case 'w':
			end = ups_autolink__www(&rewind, link, text->data + i, i, text->size - i);
			if (end > 0) {
				BUFPUTSL(ob, "<a href=\"http://");
				bufput(ob, link->data, link->size);
				BUFPUTSL(ob, "\">");
				upshtml_escape(ob, link->data, link->size);
				BUFPUTSL(ob, "</a>");
			}
			break;

		case ':':
			end = ups_autolink__url(&rewind, link, text->data + i, i, text->size - i);
			if (end > 0) {
				ob->size -= rewind;
				BUFPUTSL(ob, "<a href=\"");
				bufput(ob, link->data, link->size);
				BUFPUTSL(ob, "\">");
				upshtml_escape(ob, link->data, link->size);
				BUFPUTSL(ob, "</a>");
			}
			break;

		case '<':
			end = skip_tags(ob, text->data + i, text->size - i);
			break;

		default:
			end = 0;
			break;
		}

		if (!end)
			end = i + 1;
		else { 
			i += end;
			end = i;
		} 
	}
}


