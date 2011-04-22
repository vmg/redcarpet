/*
 * Copyright (c) 2009, Natacha Porté
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
#include "xhtml.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

struct xhtml_renderopt {
	struct {
		int header_count;
		int current_level;
	} toc_data;

	struct {
		int in_squote;
		int in_dquote;
	} quotes;

	unsigned int flags;
};

static inline void
put_scaped_char(struct buf *ob, char c)
{
	switch (c) {
		case '<': BUFPUTSL(ob, "&lt;"); break;
		case '>': BUFPUTSL(ob, "&gt;"); break;
		case '&': BUFPUTSL(ob, "&amp;"); break;
		case '"': BUFPUTSL(ob, "&quot;"); break;
		default: bufputc(ob, c); break;
	}
}

/* lus_attr_escape • copy the buffer entity-escaping '<', '>', '&' and '"' */
static void
lus_attr_escape(struct buf *ob, const char *src, size_t size)
{
	size_t  i = 0, org;
	while (i < size) {
		/* copying directly unescaped characters */
		org = i;
		while (i < size && src[i] != '<' && src[i] != '>'
		&& src[i] != '&' && src[i] != '"')
			i += 1;
		if (i > org) bufput(ob, src + org, i - org);

		/* escaping */
		if (i >= size) break;

		put_scaped_char(ob, src[i]);
		i++;
	}
}

static int
is_html_tag(struct buf *tag, const char *tagname)
{
	size_t i = 0;

	if (i < tag->size && tag->data[0] != '<')
		return 0;

	i++;

	while (i < tag->size && isspace(tag->data[i]))
		i++;

	if (i < tag->size && tag->data[i] == '/')
		i++;

	while (i < tag->size && isspace(tag->data[i]))
		i++;

	for (; i < tag->size; ++i, ++tagname) {
		if (*tagname == 0)
			break;

		if (tag->data[i] != *tagname)
			return 0;
	}

	if (i == tag->size)
		return 0;

	return (isspace(tag->data[i]) || tag->data[i] == '>');
}

/********************
 * GENERIC RENDERER *
 ********************/
static int
rndr_autolink(struct buf *ob, struct buf *link, enum mkd_autolink type, void *opaque)
{
	struct xhtml_renderopt *options = opaque;

	if (!link || !link->size)
		return 0;

	if ((options->flags & XHTML_SAFELINK) != 0 && !is_safe_link(link->data, link->size))
		return 0;

	BUFPUTSL(ob, "<a href=\"");
	if (type == MKDA_IMPLICIT_EMAIL)
		BUFPUTSL(ob, "mailto:");
	bufput(ob, link->data, link->size);
	BUFPUTSL(ob, "\">");

	if (type == MKDA_EXPLICIT_EMAIL && link->size > 7)
		lus_attr_escape(ob, link->data + 7, link->size - 7);
	else
		lus_attr_escape(ob, link->data, link->size);

	BUFPUTSL(ob, "</a>");

	return 1;
}

static void
rndr_blockcode(struct buf *ob, struct buf *text, struct buf *lang, void *opaque)
{
	static char *sh_lang = "bash";
	struct buf lang_shebang = {0, 0, 0, 0, 0};

	if (ob->size) bufputc(ob, '\n');

	/*
	 * Try to guess the language based on the shebang
	 */
	if (lang == NULL && text != NULL && text->size > 2) {
		if (bufprefix(text, "#!/usr/bin/env ") == 0) {
			size_t i = STRLEN("#!/usr/bin/env ");

			lang_shebang.data = text->data + i;
			while (i < text->size && !isspace(text->data[i])) {
				i++; lang_shebang.size++;
			}

			lang = &lang_shebang;
		} else if (bufprefix(text, "#!/bin/sh") == 0 && isspace(text->data[STRLEN("#!/bin/sh")])) {
			lang_shebang.data = sh_lang;
			lang_shebang.size = strlen(sh_lang);
			lang = &lang_shebang;
		}
	}

	if (lang && lang->size) {
		BUFPUTSL(ob, "<pre><code class=\"");
		if (lang->data[0] == '.')
			bufput(ob, lang->data + 1, lang->size - 1);
		else
			bufput(ob, lang->data, lang->size);
		BUFPUTSL(ob, "\">");
	} else
		BUFPUTSL(ob, "<pre><code>");

	if (text)
		lus_attr_escape(ob, text->data, text->size);

	BUFPUTSL(ob, "</code></pre>\n");
}

static void
rndr_blockquote(struct buf *ob, struct buf *text, void *opaque)
{
	BUFPUTSL(ob, "<blockquote>\n");
	if (text) bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</blockquote>");
}

static int
rndr_codespan(struct buf *ob, struct buf *text, void *opaque)
{
	BUFPUTSL(ob, "<code>");
	if (text) lus_attr_escape(ob, text->data, text->size);
	BUFPUTSL(ob, "</code>");
	return 1;
}

static int
rndr_strikethrough(struct buf *ob, struct buf *text, void *opaque)
{
	if (!text || !text->size)
		return 0;

	BUFPUTSL(ob, "<del>");
	bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</del>");
	return 1;
}

static int
rndr_double_emphasis(struct buf *ob, struct buf *text, void *opaque)
{
	if (!text || !text->size)
		return 0;

	BUFPUTSL(ob, "<strong>");
	bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</strong>");

	return 1;
}

static int
rndr_emphasis(struct buf *ob, struct buf *text, void *opaque)
{
	if (!text || !text->size) return 0;
	BUFPUTSL(ob, "<em>");
	if (text) bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</em>");
	return 1;
}

static void
rndr_header(struct buf *ob, struct buf *text, int level, void *opaque)
{
	struct xhtml_renderopt *options = opaque;
	
	if (ob->size)
		bufputc(ob, '\n');

	if (options->flags & XHTML_TOC) {
		bufprintf(ob, "<a name=\"toc_%d\"></a>", options->toc_data.header_count++);
	}

	bufprintf(ob, "<h%d>", level);
	if (text) bufput(ob, text->data, text->size);
	bufprintf(ob, "</h%d>\n", level);
}

static int
rndr_link(struct buf *ob, struct buf *link, struct buf *title, struct buf *content, void *opaque)
{
	struct xhtml_renderopt *options = opaque;
	
	if ((options->flags & XHTML_SAFELINK) != 0 && !is_safe_link(link->data, link->size))
		return 0;

	BUFPUTSL(ob, "<a href=\"");
	if (link && link->size) lus_attr_escape(ob, link->data, link->size);
	if (title && title->size) {
		BUFPUTSL(ob, "\" title=\"");
		lus_attr_escape(ob, title->data, title->size); }
	BUFPUTSL(ob, "\">");
	if (content && content->size) bufput(ob, content->data, content->size);
	BUFPUTSL(ob, "</a>");
	return 1;
}

static void
rndr_list(struct buf *ob, struct buf *text, int flags, void *opaque)
{
	if (ob->size) bufputc(ob, '\n');
	bufput(ob, flags & MKD_LIST_ORDERED ? "<ol>\n" : "<ul>\n", 5);
	if (text) bufput(ob, text->data, text->size);
	bufput(ob, flags & MKD_LIST_ORDERED ? "</ol>\n" : "</ul>\n", 6);
}

static void
rndr_listitem(struct buf *ob, struct buf *text, int flags, void *opaque)
{
	BUFPUTSL(ob, "<li>");
	if (text) {
		while (text->size && text->data[text->size - 1] == '\n')
			text->size -= 1;
		bufput(ob, text->data, text->size); }
	BUFPUTSL(ob, "</li>\n");
}

static void
rndr_paragraph(struct buf *ob, struct buf *text, void *opaque)
{
	struct xhtml_renderopt *options = opaque;
	size_t i = 0;

	if (ob->size) bufputc(ob, '\n');

	if (!text || !text->size)
		return;

	while (i < text->size && isspace(text->data[i])) i++;

	if (i == text->size)
		return;

	BUFPUTSL(ob, "<p>");
	if (options->flags & XHTML_HARD_WRAP) {
		size_t org;
		while (i < text->size) {
			org = i;
			while (i < text->size && text->data[i] != '\n')
				i++;

			if (i > org)
				bufput(ob, text->data + org, i - org);

			if (i >= text->size)
				break;

			BUFPUTSL(ob, "<br/>\n");
			i++;
		}
	} else {
		bufput(ob, &text->data[i], text->size - i);
	}
	BUFPUTSL(ob, "</p>\n");

	/* Close any open quotes at the end of the paragraph */
	options->quotes.in_squote = 0;
	options->quotes.in_dquote = 0;
}

static void
rndr_raw_block(struct buf *ob, struct buf *text, void *opaque)
{
	size_t org, sz;
	if (!text) return;
	sz = text->size;
	while (sz > 0 && text->data[sz - 1] == '\n') sz -= 1;
	org = 0;
	while (org < sz && text->data[org] == '\n') org += 1;
	if (org >= sz) return;
	if (ob->size) bufputc(ob, '\n');
	bufput(ob, text->data + org, sz - org);
	bufputc(ob, '\n');
}

static int
rndr_triple_emphasis(struct buf *ob, struct buf *text, void *opaque)
{
	if (!text || !text->size) return 0;
	BUFPUTSL(ob, "<strong><em>");
	bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</em></strong>");
	return 1;
}


/**********************
 * XHTML 1.0 RENDERER *
 **********************/

static void
rndr_hrule(struct buf *ob, void *opaque)
{
	if (ob->size) bufputc(ob, '\n');
	BUFPUTSL(ob, "<hr />\n");
}

static int
rndr_image(struct buf *ob, struct buf *link, struct buf *title, struct buf *alt, void *opaque)
{
	if (!link || !link->size) return 0;
	BUFPUTSL(ob, "<img src=\"");
	lus_attr_escape(ob, link->data, link->size);
	BUFPUTSL(ob, "\" alt=\"");
	if (alt && alt->size)
		lus_attr_escape(ob, alt->data, alt->size);
	if (title && title->size) {
		BUFPUTSL(ob, "\" title=\"");
		lus_attr_escape(ob, title->data, title->size); }
	BUFPUTSL(ob, "\" />");
	return 1;
}

static int
rndr_linebreak(struct buf *ob, void *opaque)
{
	BUFPUTSL(ob, "<br />\n");
	return 1;
}

static int
rndr_raw_html(struct buf *ob, struct buf *text, void *opaque)
{
	struct xhtml_renderopt *options = opaque;	
	int escape_html = 0;

	if (options->flags & XHTML_SKIP_HTML)
		escape_html = 1;

	else if ((options->flags & XHTML_SKIP_STYLE) != 0 && is_html_tag(text, "style"))
		escape_html = 1;

	else if ((options->flags & XHTML_SKIP_LINKS) != 0 && is_html_tag(text, "a"))
		escape_html = 1;

	else if ((options->flags & XHTML_SKIP_IMAGES) != 0 && is_html_tag(text, "img"))
		escape_html = 1;


	if (escape_html)
		lus_attr_escape(ob, text->data, text->size);
	else
		bufput(ob, text->data, text->size);

	return 1;
}

static void
rndr_table(struct buf *ob, struct buf *header, struct buf *body, void *opaque)
{
	if (ob->size) bufputc(ob, '\n');
	BUFPUTSL(ob, "<table><thead>\n");
	if (header)
		bufput(ob, header->data, header->size);
	BUFPUTSL(ob, "\n</thead><tbody>\n");
	if (body)
		bufput(ob, body->data, body->size);
	BUFPUTSL(ob, "\n</tbody></table>");
}

static void
rndr_tablerow(struct buf *ob, struct buf *text, void *opaque)
{
	if (ob->size) bufputc(ob, '\n');
	BUFPUTSL(ob, "<tr>\n");
	if (text)
		bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "\n</tr>");
}

static void
rndr_tablecell(struct buf *ob, struct buf *text, int align, void *opaque)
{
	if (ob->size) bufputc(ob, '\n');
	switch (align) {
	case MKD_TABLE_ALIGN_L:
		BUFPUTSL(ob, "<td align=\"left\">");
		break;

	case MKD_TABLE_ALIGN_R:
		BUFPUTSL(ob, "<td align=\"right\">");
		break;

	case MKD_TABLE_ALIGN_CENTER:
		BUFPUTSL(ob, "<td align=\"center\">");
		break;

	default:
		BUFPUTSL(ob, "<td>");
		break;
	}

	if (text)
		bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</td>");
}

static struct {
    char c0;
    const char *pattern;
    const char *entity;
    int skip;
} smartypants_subs[] = {
    { '\'', "'s>",      "&rsquo;",  0 },
    { '\'', "'t>",      "&rsquo;",  0 },
    { '\'', "'re>",     "&rsquo;",  0 },
    { '\'', "'ll>",     "&rsquo;",  0 },
    { '\'', "'ve>",     "&rsquo;",  0 },
    { '\'', "'m>",      "&rsquo;",  0 },
    { '\'', "'d>",      "&rsquo;",  0 },
    { '-',  "--",       "&mdash;",  1 },
    { '-',  "<->",      "&ndash;",  0 },
    { '.',  "...",      "&hellip;", 2 },
    { '.',  ". . .",    "&hellip;", 4 },
    { '(',  "(c)",      "&copy;",   2 },
    { '(',  "(r)",      "&reg;",    2 },
    { '(',  "(tm)",     "&trade;",  3 },
    { '3',  "<3/4>",    "&frac34;", 2 },
    { '3',  "<3/4ths>", "&frac34;", 2 },
    { '1',  "<1/2>",    "&frac12;", 2 },
    { '1',  "<1/4>",    "&frac14;", 2 },
    { '1',  "<1/4th>",  "&frac14;", 2 },
    { '&',  "&#0;",      0,       3 },
};

#define SUBS_COUNT (sizeof(smartypants_subs) / sizeof(smartypants_subs[0]))

static inline int
word_boundary(char c)
{
	return isspace(c) || ispunct(c);
}

static int
smartypants_cmpsub(const struct buf *buf, size_t start, const char *prefix)
{
	size_t i;

	if (prefix[0] == '<') {
		if (start == 0 || !word_boundary(buf->data[start - 1]))
			return 0;

		prefix++;
	}

	for (i = start; i < buf->size; ++i) {
		char c, p;

		c = tolower(buf->data[i]);
		p = *prefix++;

		if (p == 0)
			return 1;

		if (p == '>')
			return word_boundary(c);

		if (c != p)
			return 0;
	}

	return (*prefix == '>');
}

static int
smartypants_quotes(struct buf *ob, struct buf *text, size_t i, int is_open)
{
	char ent[8];

	if (is_open && i + 1 < text->size && !word_boundary(text->data[i + 1]))
		return 0;

	if (!is_open && i > 0 && !word_boundary(text->data[i - 1]))
		return 0;

	snprintf(ent, sizeof(ent), "&%c%cquo;",
		is_open ? 'r' : 'l',
		text->data[i] == '\'' ? 's' : 'd');

	bufputs(ob, ent);
	return 1;
}

static void
rndr_normal_text(struct buf *ob, struct buf *text, void *opaque)
{
	if (text)
		lus_attr_escape(ob, text->data, text->size);
}

static void
rndr_smartypants(struct buf *ob, struct buf *text, void *opaque)
{
	struct xhtml_renderopt *options = opaque;
	size_t i;

	if (!text)
		return;

	for (i = 0; i < text->size; ++i) {
		size_t sub;
		char c = text->data[i];

		for (sub = 0; sub < SUBS_COUNT; ++sub) {
			if (c == smartypants_subs[sub].c0 &&
				smartypants_cmpsub(text, i, smartypants_subs[sub].pattern)) {

				if (smartypants_subs[sub].entity)
					bufputs(ob, smartypants_subs[sub].entity);

				i += smartypants_subs[sub].skip;
				break;
			}
		}

		if (sub < SUBS_COUNT)
			continue;

		switch (c) {
		case '\"':
			if (smartypants_quotes(ob, text, i, options->quotes.in_dquote)) {
				options->quotes.in_dquote = !options->quotes.in_dquote;
				continue;
			}
			break;

		case '\'':
			if (smartypants_quotes(ob, text, i, options->quotes.in_squote)) {
				options->quotes.in_squote = !options->quotes.in_squote;
				continue;
			}
			break;
		}

		/*
		 * Copy raw character
		 */
		put_scaped_char(ob, c);
	}
}

static void
toc_header(struct buf *ob, struct buf *text, int level, void *opaque)
{
	struct xhtml_renderopt *options = opaque;

	if (level > options->toc_data.current_level) {
		if (level > 1)
			BUFPUTSL(ob, "<li>");
		BUFPUTSL(ob, "<ul>\n");
	}
	
	if (level < options->toc_data.current_level) {
		BUFPUTSL(ob, "</ul>");
		if (options->toc_data.current_level > 1)
			BUFPUTSL(ob, "</li>\n");
	}

	options->toc_data.current_level = level;

	bufprintf(ob, "<li><a href=\"#toc_%d\">", options->toc_data.header_count++);
	if (text)
		bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</a></li>\n");
}

static void
toc_finalize(struct buf *ob, void *opaque)
{
	struct xhtml_renderopt *options = opaque;

	while (options->toc_data.current_level > 1) {
		BUFPUTSL(ob, "</ul></li>\n");
		options->toc_data.current_level--;
	}

	if (options->toc_data.current_level)
		BUFPUTSL(ob, "</ul>\n");
}

void
ups_toc_renderer(struct mkd_renderer *renderer)
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
		rndr_codespan,
		rndr_double_emphasis,
		rndr_emphasis,
		NULL,
		NULL,
		NULL,
		NULL,
		rndr_triple_emphasis,
		rndr_strikethrough,

		NULL,
		NULL,

		NULL,
		toc_finalize,

		NULL
	};

	struct xhtml_renderopt *options;	
	options = calloc(1, sizeof(struct xhtml_renderopt));
	options->flags = XHTML_TOC;

	memcpy(renderer, &toc_render, sizeof(struct mkd_renderer));
	renderer->opaque = options;
}

void
ups_xhtml_renderer(struct mkd_renderer *renderer, unsigned int render_flags)
{
	static const struct mkd_renderer renderer_default = {
		rndr_blockcode,
		rndr_blockquote,
		rndr_raw_block,
		rndr_header,
		rndr_hrule,
		rndr_list,
		rndr_listitem,
		rndr_paragraph,
		rndr_table,
		rndr_tablerow,
		rndr_tablecell,

		rndr_autolink,
		rndr_codespan,
		rndr_double_emphasis,
		rndr_emphasis,
		rndr_image,
		rndr_linebreak,
		rndr_link,
		rndr_raw_html,
		rndr_triple_emphasis,
		rndr_strikethrough,

		NULL,
		rndr_normal_text,

		NULL,
		NULL,

		NULL
	};

	struct xhtml_renderopt *options;	
	options = calloc(1, sizeof(struct xhtml_renderopt));
	options->flags = render_flags;

	memcpy(renderer, &renderer_default, sizeof(struct mkd_renderer));
	renderer->opaque = options;

	if (render_flags & XHTML_SKIP_IMAGES)
		renderer->image = NULL;

	if (render_flags & XHTML_SKIP_LINKS) {
		renderer->link = NULL;
		renderer->autolink = NULL;
	}

	if (render_flags & XHTML_SMARTYPANTS)
		renderer->normal_text = rndr_smartypants;
}

void
ups_free_renderer(struct mkd_renderer *renderer)
{
	free(renderer->opaque);
}

