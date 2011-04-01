/*
 * Copyright (c) 2011, Vicent Marti
 * Copyright (c) 2009, Natacha Porté
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


static int is_safe_link(const char *link, size_t link_len)
{
	static const size_t valid_uris_count = 4;
	static const char *valid_uris[] = {
		"http:", "https:", "ftp:", "mailto:"
	};

	size_t i;

	for (i = 0; i < valid_uris_count; ++i) {
		size_t len = strlen(valid_uris[i]);

		if (link_len > len && memcmp(link, valid_uris[i], len) == 0)
			return 1;
	}

	return 0;
}

static int
put_scaped_char(struct buf *ob, char c)
{
	switch (c) {
		case '<': BUFPUTSL(ob, "&lt;"); return 1;
		case '>': BUFPUTSL(ob, "&gt;"); return 1;
		case '&': BUFPUTSL(ob, "&amp;"); return 1;
		case '"': BUFPUTSL(ob, "&quot;"); return 1;
		default: return 0;
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
	size_t i;

	for (i = 0; i < tag->size; ++i) {
		if (tagname[i] == '>')
			break;

		if (tag->data[i] != tagname[i])
			return 0;
	}

	return (i == tag->size || isspace(tag->data[i]) || tag->data[i] == '>');
}

/********************
 * GENERIC RENDERER *
 ********************/

static void
rndr_autolink2(struct buf *ob, const char *link, size_t link_size, enum mkd_autolink type)
{
	BUFPUTSL(ob, "<a href=\"");
	if (type == MKDA_IMPLICIT_EMAIL) BUFPUTSL(ob, "mailto:");
	lus_attr_escape(ob, link, link_size);
	BUFPUTSL(ob, "\">");
	if (type == MKDA_EXPLICIT_EMAIL && link_size > 7)
		lus_attr_escape(ob, link + 7, link_size - 7);
	else	lus_attr_escape(ob, link, link_size);
	BUFPUTSL(ob, "</a>");
}

static int
rndr_autolink(struct buf *ob, struct buf *link, enum mkd_autolink type, struct mkd_renderopt *options)
{
	if (!link || !link->size)
		return 0;

	if ((options->flags & RENDER_SAFELINK) != 0 && !is_safe_link(link->data, link->size))
		return 0;

	rndr_autolink2(ob, link->data, link->size, type);
	return 1;
}

static void
rndr_blockcode(struct buf *ob, struct buf *text, struct mkd_renderopt *options)
{
	if (ob->size) bufputc(ob, '\n');
	BUFPUTSL(ob, "<pre><code>");
	if (text) lus_attr_escape(ob, text->data, text->size);
	BUFPUTSL(ob, "</code></pre>\n");
}

static void
rndr_blockquote(struct buf *ob, struct buf *text, struct mkd_renderopt *options)
{
	BUFPUTSL(ob, "<blockquote>\n");
	if (text) bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</blockquote>");
}

static int
rndr_codespan(struct buf *ob, struct buf *text, struct mkd_renderopt *options)
{
	BUFPUTSL(ob, "<code>");
	if (text) lus_attr_escape(ob, text->data, text->size);
	BUFPUTSL(ob, "</code>");
	return 1;
}

static int
rndr_double_emphasis(struct buf *ob, struct buf *text, char c, struct mkd_renderopt *options)
{
	if (!text || !text->size) return 0;
	BUFPUTSL(ob, "<strong>");
	bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</strong>");
	return 1;
}

static int
rndr_emphasis(struct buf *ob, struct buf *text, char c, struct mkd_renderopt *options)
{
	if (!text || !text->size) return 0;
	BUFPUTSL(ob, "<em>");
	if (text) bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</em>");
	return 1;
}

static void
rndr_header(struct buf *ob, struct buf *text, int level, struct mkd_renderopt *options)
{
	if (ob->size) bufputc(ob, '\n');
	bufprintf(ob, "<h%d>", level);
	if (text) bufput(ob, text->data, text->size);
	bufprintf(ob, "</h%d>\n", level);
}

static int
rndr_link(struct buf *ob, struct buf *link, struct buf *title, struct buf *content, struct mkd_renderopt *options)
{
	if ((options->flags & RENDER_SAFELINK) != 0 && !is_safe_link(link->data, link->size))
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
rndr_list(struct buf *ob, struct buf *text, int flags, struct mkd_renderopt *options)
{
	if (ob->size) bufputc(ob, '\n');
	bufput(ob, flags & MKD_LIST_ORDERED ? "<ol>\n" : "<ul>\n", 5);
	if (text) bufput(ob, text->data, text->size);
	bufput(ob, flags & MKD_LIST_ORDERED ? "</ol>\n" : "</ul>\n", 6);
}

static void
rndr_listitem(struct buf *ob, struct buf *text, int flags, struct mkd_renderopt *options)
{
	BUFPUTSL(ob, "<li>");
	if (text) {
		while (text->size && text->data[text->size - 1] == '\n')
			text->size -= 1;
		bufput(ob, text->data, text->size); }
	BUFPUTSL(ob, "</li>\n");
}

static void
rndr_paragraph(struct buf *ob, struct buf *text, struct mkd_renderopt *options)
{
	size_t i = 0;

	if (ob->size) bufputc(ob, '\n');

	if (!text || !text->size)
		return;

	while (i < text->size && isspace(text->data[i])) i++;

	if (i < text->size) {
		BUFPUTSL(ob, "<p>");
		bufput(ob, &text->data[i], text->size - i);
		BUFPUTSL(ob, "</p>\n");
	}
}

static void
rndr_raw_block(struct buf *ob, struct buf *text, struct mkd_renderopt *options)
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
rndr_triple_emphasis(struct buf *ob, struct buf *text, char c, struct mkd_renderopt *options)
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
rndr_hrule(struct buf *ob, struct mkd_renderopt *options)
{
	if (ob->size) bufputc(ob, '\n');
	BUFPUTSL(ob, "<hr />\n");
}

static int
rndr_image(struct buf *ob, struct buf *link, struct buf *title, struct buf *alt, struct mkd_renderopt *options)
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
rndr_linebreak(struct buf *ob, struct mkd_renderopt *options)
{
	BUFPUTSL(ob, "<br />\n");
	return 1;
}

static int
rndr_raw_html(struct buf *ob, struct buf *text, struct mkd_renderopt *options)
{
	int escape_html = 0;

	if (options->flags & RENDER_SKIP_HTML)
		escape_html = 1;

	else if ((options->flags & RENDER_SKIP_STYLE) != 0 && is_html_tag(text, "<style>"))
		escape_html = 1;

	else if ((options->flags & RENDER_SKIP_LINKS) != 0 && is_html_tag(text, "<a>"))
		escape_html = 1;

	else if ((options->flags & RENDER_SKIP_IMAGES) != 0 && is_html_tag(text, "<img>"))
		escape_html = 1;


	if (escape_html)
		lus_attr_escape(ob, text->data, text->size);
	else
		bufput(ob, text->data, text->size);

	return 1;
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

static inline
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
rndr_normal_text(struct buf *ob, struct buf *text, struct mkd_renderopt *options)
{
	size_t i;
	int open_single = 0, open_double = 0, open_tag = 0;

	int autolink = (options->flags & RENDER_AUTOLINK);
	int smartypants = (options->flags & RENDER_SMARTYPANTS);

	if (!text)
		return;

	if (!autolink && !smartypants) {
		lus_attr_escape(ob, text->data, text->size);
		return;
	}

	for (i = 0; i < text->size; ++i) {
		size_t sub;
		char c = text->data[i];

		/*
		 * Autolinking
		 */
		if (autolink) {
			/* Autolinking is not standarized in the Markdown spec.
			 * We only check for links after special characters, i.e.
			 * immediately after a space or a parenthesis */
			if (i == 0 || (isspace(text->data[i - 1]) || text->data[i - 1] == '(') &&
				is_safe_link(text->data + i, text->size - i)) {
				size_t j = i + i;

				while (j < text->size && !isspace(text->data[j]))
					j++;

				rndr_autolink2(ob, &text->data[i], j - i, MKDA_NORMAL);
				i = j;
				continue;
			}
		}

		/*
		 * Smartypants subsitutions
		 */
		if (smartypants) {
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
			case '<':
				open_tag = 1;
				break;

			case '>':
				open_tag = 0;
				break;

#if 0
			/*
			 * FIXME: this is bongos.
			 *
			 * The markdown spec defines that code blocks can be delimited
			 * by more than one backtick, e.g.
			 *
			 *		``There is a literal backtick (`) here.``
			 *		<p><code>There is a literal backtick (`) here.</code></p>
			 *
			 * Obviously, there's no way to differentiate between the start
			 * of a code block and the start of a quoted string for smartypants
			 *
			 * Look at this piece of Python code:
			 *
			 *		``result = ''.join(['this', 'is', 'bongos'])``
			 *
			 * This MD expression is clearly ambiguous since it can be parsed as:
			 *
			 *		<p>&ldquo;result = &rdquo;.join ...</p>
			 *
			 * Or also as:
			 *
			 *		<p><code>result = ''.join(['this', 'is', 'bongos'])</code></p>
			 *
			 * Fuck everything about this. This is temporarily disabled, because at GitHub
			 * it's probably smarter to prioritize code blocks than pretty cutesy punctuation.
			 *
			 * The equivalent closing tag for the (``), ('') has also been disabled, because
			 * it makes no sense to have closing tags without opening tags.
			 */
			case '`':
				if (open_tag == 0) {
					if (i + 1 < text->size && text->data[i + 1] == '`') {
						BUFPUTSL(ob, "&ldquo;"); i++;
						continue;
					}
				}
				break;
#endif

			case '\"':
				if (open_tag == 0) {
					if (smartypants_quotes(ob, text, i, open_double)) {
						open_double = !open_double;
						continue;
					}
				}
				break;

			case '\'':
				if (open_tag == 0) {

#if 0 /* temporarily disabled, see previous comment */
					if (i + 1 < text->size && text->data[i + 1] == '\'') {
						BUFPUTSL(ob, "&rdquo;"); i++;
						continue;
					} 
#endif

					if (smartypants_quotes(ob, text, i, open_single)) {
						open_single = !open_single;
						continue;
					}
				}
				break;
			}
		}

		/*
		 * Copy raw character
		 */
		if (!put_scaped_char(ob, c))
			bufputc(ob, c);
	}
}

void
init_renderer(struct mkd_renderer *renderer,
		unsigned int render_flags, void *opaque,
		unsigned int parser_flags, int recursion_depth)
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

		rndr_autolink,
		rndr_codespan,
		rndr_double_emphasis,
		rndr_emphasis,
		rndr_image,
		rndr_linebreak,
		rndr_link,
		rndr_raw_html,
		rndr_triple_emphasis,

		NULL,
		rndr_normal_text,

		"*_",

		{ NULL, 0 },
		{ 0, 0 },
	};

	memcpy(renderer, &renderer_default, sizeof(struct mkd_renderer));

	if (render_flags & RENDER_SKIP_IMAGES)
		renderer->image = NULL;

	if (render_flags & RENDER_SKIP_LINKS) {
		renderer->link = NULL;
		renderer->autolink = NULL;
	}

	renderer->parser_options.recursion_depth = recursion_depth;
	renderer->parser_options.flags = parser_flags;

	renderer->render_options.opaque = opaque;
	renderer->render_options.flags = render_flags;
}

