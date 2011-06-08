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
#include "html.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

struct html_renderopt {
	struct {
		int header_count;
		int current_level;
	} toc_data;

	unsigned int flags;
	const char *close_tag;
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

/* upshtml_escape • copy the buffer entity-escaping '<', '>', '&' and '"' */
void
upshtml_escape(struct buf *ob, const char *src, size_t size)
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
	struct html_renderopt *options = opaque;

	if (!link || !link->size)
		return 0;

	if ((options->flags & HTML_SAFELINK) != 0 &&
		!is_safe_link(link->data, link->size) &&
		type != MKDA_EMAIL)
		return 0;

	BUFPUTSL(ob, "<a href=\"");
	if (type == MKDA_EMAIL)
		BUFPUTSL(ob, "mailto:");
	bufput(ob, link->data, link->size);
	BUFPUTSL(ob, "\">");

	/*
	 * Pretty printing: if we get an email address as
	 * an actual URI, e.g. `mailto:foo@bar.com`, we don't
	 * want to print the `mailto:` prefix
	 */
	if (bufprefix(link, "mailto:") == 0) {
		upshtml_escape(ob, link->data + 7, link->size - 7);
	} else {
		upshtml_escape(ob, link->data, link->size);
	}

	BUFPUTSL(ob, "</a>");

	return 1;
}

static void
rndr_blockcode(struct buf *ob, struct buf *text, struct buf *lang, void *opaque)
{
	if (ob->size) bufputc(ob, '\n');

	if (lang && lang->size) {
		size_t i, cls;
		BUFPUTSL(ob, "<pre><code class=\"");

		for (i = 0, cls = 0; i < lang->size; ++i, ++cls) {
			while (i < lang->size && isspace(lang->data[i]))
				i++;

			if (i < lang->size) {
				size_t org = i;
				while (i < lang->size && !isspace(lang->data[i]))
					i++;

				if (lang->data[org] == '.')
					org++;

				if (cls) bufputc(ob, ' ');
				upshtml_escape(ob, lang->data + org, i - org);
			}
		}

		BUFPUTSL(ob, "\">");
	} else
		BUFPUTSL(ob, "<pre><code>");

	if (text)
		upshtml_escape(ob, text->data, text->size);

	BUFPUTSL(ob, "</code></pre>\n");
}

/*
 * GitHub style code block:
 *
 *		<pre lang="LANG"><code>
 *		...
 *		</pre></code>
 *
 * Unlike other parsers, we store the language identifier in the <pre>,
 * and don't let the user generate custom classes.
 *
 * The language identifier in the <pre> block gets postprocessed and all
 * the code inside gets syntax highlighted with Pygments. This is much safer
 * than letting the user specify a CSS class for highlighting.
 *
 * Note that we only generate HTML for the first specifier.
 * E.g.
 *		~~~~ {.python .numbered}	=>	<pre lang="python"><code>
 */
static void
rndr_blockcode_github(struct buf *ob, struct buf *text, struct buf *lang, void *opaque)
{
	if (ob->size) bufputc(ob, '\n');

	if (lang && lang->size) {
		size_t i = 0;
		BUFPUTSL(ob, "<pre lang=\"");

		while (i < lang->size && !isspace(lang->data[i]))
			i++;

		if (lang->data[0] == '.')
			upshtml_escape(ob, lang->data + 1, i - 1);
		else
			upshtml_escape(ob, lang->data, i);

		BUFPUTSL(ob, "\"><code>");
	} else
		BUFPUTSL(ob, "<pre><code>");

	if (text)
		upshtml_escape(ob, text->data, text->size);

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
	if (text) upshtml_escape(ob, text->data, text->size);
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
	struct html_renderopt *options = opaque;
	
	if (ob->size)
		bufputc(ob, '\n');

	if (options->flags & HTML_TOC)
		bufprintf(ob, "<h%d id=\"toc_%d\">", level, options->toc_data.header_count++);
	else
		bufprintf(ob, "<h%d>", level);

	if (text) bufput(ob, text->data, text->size);
	bufprintf(ob, "</h%d>\n", level);
}

static int
rndr_link(struct buf *ob, struct buf *link, struct buf *title, struct buf *content, void *opaque)
{
	struct html_renderopt *options = opaque;
	
	if ((options->flags & HTML_SAFELINK) != 0 && !is_safe_link(link->data, link->size))
		return 0;

	BUFPUTSL(ob, "<a href=\"");
	if (link && link->size) bufput(ob, link->data, link->size);
	if (title && title->size) {
		BUFPUTSL(ob, "\" title=\"");
		upshtml_escape(ob, title->data, title->size); }
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
	struct html_renderopt *options = opaque;
	size_t i = 0;

	if (ob->size) bufputc(ob, '\n');

	if (!text || !text->size)
		return;

	while (i < text->size && isspace(text->data[i])) i++;

	if (i == text->size)
		return;

	BUFPUTSL(ob, "<p>");
	if (options->flags & HTML_HARD_WRAP) {
		size_t org;
		while (i < text->size) {
			org = i;
			while (i < text->size && text->data[i] != '\n')
				i++;

			if (i > org)
				bufput(ob, text->data + org, i - org);

			if (i >= text->size)
				break;

			BUFPUTSL(ob, "<br");
			bufputs(ob, options->close_tag);
			i++;
		}
	} else {
		bufput(ob, &text->data[i], text->size - i);
	}
	BUFPUTSL(ob, "</p>\n");
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

static void
rndr_hrule(struct buf *ob, void *opaque)
{
	struct html_renderopt *options = opaque;	
	if (ob->size) bufputc(ob, '\n');
	BUFPUTSL(ob, "<hr");
	bufputs(ob, options->close_tag);
}

static int
rndr_image(struct buf *ob, struct buf *link, struct buf *title, struct buf *alt, void *opaque)
{
	struct html_renderopt *options = opaque;	
	if (!link || !link->size) return 0;
	BUFPUTSL(ob, "<img src=\"");
	upshtml_escape(ob, link->data, link->size);
	BUFPUTSL(ob, "\" alt=\"");
	if (alt && alt->size)
		upshtml_escape(ob, alt->data, alt->size);
	if (title && title->size) {
		BUFPUTSL(ob, "\" title=\"");
		upshtml_escape(ob, title->data, title->size); }

	bufputc(ob, '"');
	bufputs(ob, options->close_tag);
	return 1;
}

static int
rndr_linebreak(struct buf *ob, void *opaque)
{
	struct html_renderopt *options = opaque;	
	BUFPUTSL(ob, "<br");
	bufputs(ob, options->close_tag);
	return 1;
}

static int
rndr_raw_html(struct buf *ob, struct buf *text, void *opaque)
{
	struct html_renderopt *options = opaque;	

	if ((options->flags & HTML_SKIP_HTML) != 0)
		return 1;

	if ((options->flags & HTML_SKIP_STYLE) != 0 && is_html_tag(text, "style"))
		return 1;

	if ((options->flags & HTML_SKIP_LINKS) != 0 && is_html_tag(text, "a"))
		return 1;

	if ((options->flags & HTML_SKIP_IMAGES) != 0 && is_html_tag(text, "img"))
		return 1;

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

static void
rndr_normal_text(struct buf *ob, struct buf *text, void *opaque)
{
	if (text)
		upshtml_escape(ob, text->data, text->size);
}

static void
toc_header(struct buf *ob, struct buf *text, int level, void *opaque)
{
	struct html_renderopt *options = opaque;

	while (level > options->toc_data.current_level) {
		if (options->toc_data.current_level > 0)
			BUFPUTSL(ob, "<li>");
		BUFPUTSL(ob, "<ul>\n");
		options->toc_data.current_level++;
	}

	while (level < options->toc_data.current_level) {
		BUFPUTSL(ob, "</ul>");
		if (options->toc_data.current_level > 1)
			BUFPUTSL(ob, "</li>\n");
		options->toc_data.current_level--;
	}

	bufprintf(ob, "<li><a href=\"#toc_%d\">", options->toc_data.header_count++);
	if (text)
		bufput(ob, text->data, text->size);
	BUFPUTSL(ob, "</a></li>\n");
}

static void
toc_finalize(struct buf *ob, void *opaque)
{
	struct html_renderopt *options = opaque;

	while (options->toc_data.current_level > 1) {
		BUFPUTSL(ob, "</ul></li>\n");
		options->toc_data.current_level--;
	}

	if (options->toc_data.current_level)
		BUFPUTSL(ob, "</ul>\n");
}

void
upshtml_toc_renderer(struct mkd_renderer *renderer)
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

	struct html_renderopt *options;	
	options = calloc(1, sizeof(struct html_renderopt));
	options->flags = HTML_TOC;

	memcpy(renderer, &toc_render, sizeof(struct mkd_renderer));
	renderer->opaque = options;
}

void
upshtml_renderer(struct mkd_renderer *renderer, unsigned int render_flags)
{
	static const char *xhtml_close = "/>\n";
	static const char *html_close = ">\n";

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

	struct html_renderopt *options;	
	options = calloc(1, sizeof(struct html_renderopt));
	options->flags = render_flags;
	options->close_tag = (render_flags & HTML_USE_XHTML) ? xhtml_close : html_close;

	memcpy(renderer, &renderer_default, sizeof(struct mkd_renderer));
	renderer->opaque = options;

	if (render_flags & HTML_SKIP_IMAGES)
		renderer->image = NULL;

	if (render_flags & HTML_SKIP_LINKS) {
		renderer->link = NULL;
		renderer->autolink = NULL;
	}

	if (render_flags & HTML_SKIP_HTML)
		renderer->blockhtml = NULL;

	if (render_flags & HTML_GITHUB_BLOCKCODE)
		renderer->blockcode = rndr_blockcode_github;
}

void
upshtml_free_renderer(struct mkd_renderer *renderer)
{
	free(renderer->opaque);
}

