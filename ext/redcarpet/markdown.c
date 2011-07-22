/* markdown.c - generic markdown parser */

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
#include "array.h"

#include <assert.h>
#include <string.h>
//#include <strings.h> /* for strncasecmp */
#include <ctype.h>
#include <stdio.h>

#define BUFFER_BLOCK 0
#define BUFFER_SPAN 1

#define MKD_LI_END 8	/* internal list flag */

/***************
 * LOCAL TYPES *
 ***************/

/* link_ref • reference to a link */
struct link_ref {
	struct buf *id;
	struct buf *link;
	struct buf *title;
};

/* char_trigger • function pointer to render active chars */
/*   returns the number of chars taken care of */
/*   data is the pointer of the beginning of the span */
/*   offset is the number of valid chars before data */
struct render;
typedef size_t
(*char_trigger)(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size);

static size_t char_emphasis(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size);
static size_t char_linebreak(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size);
static size_t char_codespan(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size);
static size_t char_escape(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size);
static size_t char_entity(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size);
static size_t char_langle_tag(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size);
static size_t char_autolink_url(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size);
static size_t char_autolink_email(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size);
static size_t char_autolink_www(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size);
static size_t char_link(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size);
static size_t char_superscript(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size);

enum markdown_char_t {
	MD_CHAR_NONE = 0,
	MD_CHAR_EMPHASIS,
	MD_CHAR_CODESPAN,
	MD_CHAR_LINEBREAK,
	MD_CHAR_LINK,
	MD_CHAR_LANGLE,
	MD_CHAR_ESCAPE,
	MD_CHAR_ENTITITY,
	MD_CHAR_AUTOLINK_URL,
	MD_CHAR_AUTOLINK_EMAIL,
	MD_CHAR_AUTOLINK_WWW,
	MD_CHAR_SUPERSCRIPT,
};

static char_trigger markdown_char_ptrs[] = {
	NULL,
	&char_emphasis,
	&char_codespan,
	&char_linebreak,
	&char_link,
	&char_langle_tag,
	&char_escape,
	&char_entity,
	&char_autolink_url,
	&char_autolink_email,
	&char_autolink_www,
	&char_superscript,
};

/* render • structure containing one particular render */
struct render {
	struct mkd_renderer	make;
	struct array refs;
	char active_char[256];
	struct parray work_bufs[2];
	unsigned int ext_flags;
	size_t max_nesting;
};

/* html_tag • structure for quick HTML tag search (inspired from discount) */
struct html_tag {
	const char *text;
	size_t size;
};

static inline struct buf *
rndr_newbuf(struct render *rndr, int type)
{
	static const size_t buf_size[2] = {256, 64};
	struct buf *work = NULL;
	struct parray *queue = &rndr->work_bufs[type];

	if (queue->size < queue->asize) {
		work = queue->item[queue->size++];
		work->size = 0;
	} else {
		work = bufnew(buf_size[type]);
		parr_push(queue, work);
	}

	return work;
}

static inline void
rndr_popbuf(struct render *rndr, int type)
{
	rndr->work_bufs[type].size--;
}

/********************
 * GLOBAL VARIABLES *
 ********************/

/* block_tags • recognised block tags, sorted by cmp_html_tag */
static struct html_tag block_tags[] = {
/*0*/	{ "p",		1 },
	{ "dl",		2 },
	{ "h1",		2 },
	{ "h2",		2 },
	{ "h3",		2 },
	{ "h4",		2 },
	{ "h5",		2 },
	{ "h6",		2 },
	{ "ol",		2 },
	{ "ul",		2 },
	{ "del",	3 }, /* 10 */
	{ "div",	3 },
	{ "ins",	3 }, /* 12 */
	{ "pre",	3 },
	{ "form",	4 },
	{ "math",	4 },
	{ "table",	5 },
	{ "figure",	6 },
	{ "iframe",	6 },
	{ "script",	6 },
	{ "fieldset",	8 },
	{ "noscript",	8 },
	{ "blockquote",	10 }
};

#define INS_TAG (block_tags + 12)
#define DEL_TAG (block_tags + 10)

/***************************
 * HELPER FUNCTIONS *
 ***************************/
static void
unscape_text(struct buf *ob, struct buf *src)
{
	size_t i = 0, org;
	while (i < src->size) {
		org = i;
		while (i < src->size && src->data[i] != '\\')
			i++;

		if (i > org)
			bufput(ob, src->data + org, i - org);

		if (i + 1 >= src->size)
			break;

		bufputc(ob, src->data[i + 1]);
		i += 2;
	}
}

/* cmp_link_ref • comparison function for link_ref sorted arrays */
static int
cmp_link_ref(void *key, void *array_entry)
{
	struct link_ref *lr = array_entry;
	return bufcasecmp(key, lr->id);
}

/* cmp_link_ref_sort • comparison function for link_ref qsort */
static int
cmp_link_ref_sort(const void *a, const void *b)
{
	const struct link_ref *lra = a;
	const struct link_ref *lrb = b;
	return bufcasecmp(lra->id, lrb->id);
}

/* cmp_html_tag • comparison function for bsearch() (stolen from discount) */
static int
cmp_html_tag(const void *a, const void *b)
{
	const struct html_tag *hta = a;
	const struct html_tag *htb = b;
	if (hta->size != htb->size) return (int)(hta->size - htb->size);
	return strncasecmp(hta->text, htb->text, hta->size);
}


/* find_block_tag • returns the current block tag */
static struct html_tag *
find_block_tag(char *data, size_t size)
{
	size_t i = 0;
	struct html_tag key;

	/* looking for the word end */
	while (i < size && ((data[i] >= '0' && data[i] <= '9')
				|| (data[i] >= 'A' && data[i] <= 'Z')
				|| (data[i] >= 'a' && data[i] <= 'z')))
		i++;
	if (i >= size) return 0;

	/* binary search of the tag */
	key.text = data;
	key.size = i;
	return bsearch(&key, block_tags,
				sizeof block_tags / sizeof block_tags[0],
				sizeof block_tags[0], cmp_html_tag);
}

/****************************
 * INLINE PARSING FUNCTIONS *
 ****************************/

/* is_mail_autolink • looks for the address part of a mail autolink and '>' */
/* this is less strict than the original markdown e-mail address matching */
static size_t
is_mail_autolink(char *data, size_t size)
{
	size_t i = 0, nb = 0;

	/* address is assumed to be: [-@._a-zA-Z0-9]+ with exactly one '@' */
	for (i = 0; i < size; ++i) {
		if (isalnum(data[i]))
			continue;

		switch (data[i]) {
			case '@':
				nb++;

			case '-':
			case '.':
			case '_':
				break;

			case '>':
				return (nb == 1) ? i + 1 : 0;

			default:
				return 0;
		}
	}

	return 0;
}

/* tag_length • returns the length of the given tag, or 0 is it's not valid */
static size_t
tag_length(char *data, size_t size, enum mkd_autolink *autolink)
{
	size_t i, j;

	/* a valid tag can't be shorter than 3 chars */
	if (size < 3) return 0;

	/* begins with a '<' optionally followed by '/', followed by letter or number */
	if (data[0] != '<') return 0;
	i = (data[1] == '/') ? 2 : 1;

	if (!isalnum(data[i]))
		return 0;

	/* scheme test */
	*autolink = MKDA_NOT_AUTOLINK;

	/* try to find the beginning of an URI */
	while (i < size && (isalnum(data[i]) || data[i] == '.' || data[i] == '+' || data[i] == '-'))
		i++;

	if (i > 1 && data[i] == '@') {
		if ((j = is_mail_autolink(data + i, size - i)) != 0) {
			*autolink = MKDA_EMAIL;
			return i + j;
		}
	}

	if (i > 2 && data[i] == ':') {
		*autolink = MKDA_NORMAL;
		i++;
	}

	/* completing autolink test: no whitespace or ' or " */
	if (i >= size)
		*autolink = MKDA_NOT_AUTOLINK;

	else if (*autolink) {
		j = i;

		while (i < size) {
			if (data[i] == '\\') i += 2;
			else if (data[i] == '>' || data[i] == '\'' ||
					data[i] == '"' || isspace(data[i])) break;
			else i++;
		}

		if (i >= size) return 0;
		if (i > j && data[i] == '>') return i + 1;
		/* one of the forbidden chars has been found */
		*autolink = MKDA_NOT_AUTOLINK;
	}

	/* looking for sometinhg looking like a tag end */
	while (i < size && data[i] != '>') i++;
	if (i >= size) return 0;
	return i + 1;
}

/* parse_inline • parses inline markdown elements */
static void
parse_inline(struct buf *ob, struct render *rndr, char *data, size_t size)
{
	size_t i = 0, end = 0;
	char action = 0;
	struct buf work = { 0, 0, 0, 0, 0 };

	if (rndr->work_bufs[BUFFER_SPAN].size +
		rndr->work_bufs[BUFFER_BLOCK].size > (int)rndr->max_nesting)
		return;

	while (i < size) {
		/* copying inactive chars into the output */
		while (end < size && (action = rndr->active_char[(unsigned char)data[end]]) == 0) {
			end++;
		}

		if (rndr->make.normal_text) {
			work.data = data + i;
			work.size = end - i;
			rndr->make.normal_text(ob, &work, rndr->make.opaque);
		}
		else
			bufput(ob, data + i, end - i);

		if (end >= size) break;
		i = end;

		/* calling the trigger */
		end = markdown_char_ptrs[(int)action](ob, rndr, data + i, i, size - i);
		if (!end) /* no action from the callback */
			end = i + 1;
		else { 
			i += end;
			end = i;
		} 
	}
}

/* find_emph_char • looks for the next emph char, skipping other constructs */
static size_t
find_emph_char(char *data, size_t size, char c)
{
	size_t i = 1;

	while (i < size) {
		while (i < size && data[i] != c && data[i] != '`' && data[i] != '[')
			i++;

		if (i == size)
			return 0;

		if (data[i] == c)
			return i;

		/* not counting escaped chars */
		if (i && data[i - 1] == '\\') {
			i++; continue;
		}

		/* skipping a code span */
		if (data[i] == '`') {
			size_t tmp_i = 0;

			i++;
			while (i < size && data[i] != '`') {
				if (!tmp_i && data[i] == c) tmp_i = i;
				i++; 
			}

			if (i >= size)
				return tmp_i;

			i++; 
		}
		/* skipping a link */
		else if (data[i] == '[') {
			size_t tmp_i = 0;
			char cc;

			i++;
			while (i < size && data[i] != ']') {
				if (!tmp_i && data[i] == c) tmp_i = i;
				i++;
			}

			i++;
			while (i < size && (data[i] == ' ' || data[i] == '\t' || data[i] == '\n'))
				i++;

			if (i >= size)
				return tmp_i;

			if (data[i] != '[' && data[i] != '(') { /* not a link*/
				if (tmp_i) return tmp_i;
				else continue;
			}

			cc = data[i];
			i++;
			while (i < size && data[i] != cc) {
				if (!tmp_i && data[i] == c) tmp_i = i;
				i++;
			}

			if (i >= size)
				return tmp_i;

			i++;
		}
	}

	return 0;
}

/* parse_emph1 • parsing single emphase */
/* closed by a symbol not preceded by whitespace and not followed by symbol */
static size_t
parse_emph1(struct buf *ob, struct render *rndr, char *data, size_t size, char c)
{
	size_t i = 0, len;
	struct buf *work = 0;
	int r;

	if (!rndr->make.emphasis) return 0;

	/* skipping one symbol if coming from emph3 */
	if (size > 1 && data[0] == c && data[1] == c) i = 1;

	while (i < size) {
		len = find_emph_char(data + i, size - i, c);
		if (!len) return 0;
		i += len;
		if (i >= size) return 0;

		if (i + 1 < size && data[i + 1] == c) {
			i++;
			continue;
		}

		if (data[i] == c && !isspace(data[i - 1])) {

			if (rndr->ext_flags & MKDEXT_NO_INTRA_EMPHASIS) {
				if (!(i + 1 == size || isspace(data[i + 1]) || ispunct(data[i + 1])))
					continue;
			}

			work = rndr_newbuf(rndr, BUFFER_SPAN);
			parse_inline(work, rndr, data, i);
			r = rndr->make.emphasis(ob, work, rndr->make.opaque);
			rndr_popbuf(rndr, BUFFER_SPAN);
			return r ? i + 1 : 0;
		}
	}

	return 0;
}

/* parse_emph2 • parsing single emphase */
static size_t
parse_emph2(struct buf *ob, struct render *rndr, char *data, size_t size, char c)
{
	int (*render_method)(struct buf *ob, struct buf *text, void *opaque);
	size_t i = 0, len;
	struct buf *work = 0;
	int r;

	render_method = (c == '~') ? rndr->make.strikethrough : rndr->make.double_emphasis;

	if (!render_method)
		return 0;
	
	while (i < size) {
		len = find_emph_char(data + i, size - i, c);
		if (!len) return 0;
		i += len;

		if (i + 1 < size && data[i] == c && data[i + 1] == c && i && !isspace(data[i - 1])) {
			work = rndr_newbuf(rndr, BUFFER_SPAN);
			parse_inline(work, rndr, data, i);
			r = render_method(ob, work, rndr->make.opaque);
			rndr_popbuf(rndr, BUFFER_SPAN);
			return r ? i + 2 : 0;
		}
		i++;
	}
	return 0;
}

/* parse_emph3 • parsing single emphase */
/* finds the first closing tag, and delegates to the other emph */
static size_t
parse_emph3(struct buf *ob, struct render *rndr, char *data, size_t size, char c)
{
	size_t i = 0, len;
	int r;

	while (i < size) {
		len = find_emph_char(data + i, size - i, c);
		if (!len) return 0;
		i += len;

		/* skip whitespace preceded symbols */
		if (data[i] != c || isspace(data[i - 1]))
			continue;

		if (i + 2 < size && data[i + 1] == c && data[i + 2] == c && rndr->make.triple_emphasis) {
			/* triple symbol found */
			struct buf *work = rndr_newbuf(rndr, BUFFER_SPAN);

			parse_inline(work, rndr, data, i);
			r = rndr->make.triple_emphasis(ob, work, rndr->make.opaque);
			rndr_popbuf(rndr, BUFFER_SPAN);
			return r ? i + 3 : 0;

		} else if (i + 1 < size && data[i + 1] == c) {
			/* double symbol found, handing over to emph1 */
			len = parse_emph1(ob, rndr, data - 2, size + 2, c);
			if (!len) return 0;
			else return len - 2;

		} else {
			/* single symbol found, handing over to emph2 */
			len = parse_emph2(ob, rndr, data - 1, size + 1, c);
			if (!len) return 0;
			else return len - 1;
		}
	}
	return 0; 
}

/* char_emphasis • single and double emphasis parsing */
static size_t
char_emphasis(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size)
{
	char c = data[0];
	size_t ret;

	if (size > 2 && data[1] != c) {
		/* whitespace cannot follow an opening emphasis;
		 * strikethrough only takes two characters '~~' */
		if (c == '~' || isspace(data[1]) || (ret = parse_emph1(ob, rndr, data + 1, size - 1, c)) == 0)
			return 0;

		return ret + 1;
	}

	if (size > 3 && data[1] == c && data[2] != c) {
		if (isspace(data[2]) || (ret = parse_emph2(ob, rndr, data + 2, size - 2, c)) == 0)
			return 0;

		return ret + 2;
	}

	if (size > 4 && data[1] == c && data[2] == c && data[3] != c) {
		if (c == '~' || isspace(data[3]) || (ret = parse_emph3(ob, rndr, data + 3, size - 3, c)) == 0)
			return 0;

		return ret + 3;
	}

	return 0; 
}


/* char_linebreak • '\n' preceded by two spaces (assuming linebreak != 0) */
static size_t
char_linebreak(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size)
{
	if (offset < 2 || data[-1] != ' ' || data[-2] != ' ')
		return 0;

	/* removing the last space from ob and rendering */
	while (ob->size && ob->data[ob->size - 1] == ' ')
		ob->size--;

	return rndr->make.linebreak(ob, rndr->make.opaque) ? 1 : 0;
}


/* char_codespan • '`' parsing a code span (assuming codespan != 0) */
static size_t
char_codespan(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size)
{
	size_t end, nb = 0, i, f_begin, f_end;

	/* counting the number of backticks in the delimiter */
	while (nb < size && data[nb] == '`')
		nb++;

	/* finding the next delimiter */
	i = 0;
	for (end = nb; end < size && i < nb; end++) {
		if (data[end] == '`') i++;
		else i = 0;
	}

	if (i < nb && end >= size)
		return 0; /* no matching delimiter */

	/* trimming outside whitespaces */
	f_begin = nb;
	while (f_begin < end && (data[f_begin] == ' ' || data[f_begin] == '\t'))
		f_begin++;

	f_end = end - nb;
	while (f_end > nb && (data[f_end-1] == ' ' || data[f_end-1] == '\t'))
		f_end--;

	/* real code span */
	if (f_begin < f_end) {
		struct buf work = { data + f_begin, f_end - f_begin, 0, 0, 0 };
		if (!rndr->make.codespan(ob, &work, rndr->make.opaque))
			end = 0;
	} else {
		if (!rndr->make.codespan(ob, 0, rndr->make.opaque))
			end = 0;
	}

	return end;
}


/* char_escape • '\\' backslash escape */
static size_t
char_escape(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size)
{
	static const char *escape_chars = "\\`*_{}[]()#+-.!:|&<>";
	struct buf work = { 0, 0, 0, 0, 0 };

	if (size > 1) {
		if (strchr(escape_chars, data[1]) == NULL)
			return 0;

		if (rndr->make.normal_text) {
			work.data = data + 1;
			work.size = 1;
			rndr->make.normal_text(ob, &work, rndr->make.opaque);
		}
		else bufputc(ob, data[1]);
	}

	return 2;
}

/* char_entity • '&' escaped when it doesn't belong to an entity */
/* valid entities are assumed to be anything matching &#?[A-Za-z0-9]+; */
static size_t
char_entity(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size)
{
	size_t end = 1;
	struct buf work;

	if (end < size && data[end] == '#')
		end++;

	while (end < size && isalnum(data[end]))
		end++;

	if (end < size && data[end] == ';')
		end++; /* real entity */
	else
		return 0; /* lone '&' */

	if (rndr->make.entity) {
		work.data = data;
		work.size = end;
		rndr->make.entity(ob, &work, rndr->make.opaque);
	}
	else bufput(ob, data, end);

	return end;
}

/* char_langle_tag • '<' when tags or autolinks are allowed */
static size_t
char_langle_tag(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size)
{
	enum mkd_autolink altype = MKDA_NOT_AUTOLINK;
	size_t end = tag_length(data, size, &altype);
	struct buf work = { data, end, 0, 0, 0 };
	int ret = 0;

	if (end > 2) {
		if (rndr->make.autolink && altype != MKDA_NOT_AUTOLINK) {
			struct buf *u_link = rndr_newbuf(rndr, BUFFER_SPAN);
			work.data = data + 1;
			work.size = end - 2;
			unscape_text(u_link, &work);
			ret = rndr->make.autolink(ob, u_link, altype, rndr->make.opaque);
			rndr_popbuf(rndr, BUFFER_SPAN);
		}
		else if (rndr->make.raw_html_tag)
			ret = rndr->make.raw_html_tag(ob, &work, rndr->make.opaque);
	}

	if (!ret) return 0;
	else return end;
}

static size_t
char_autolink_www(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size)
{
	struct buf *link, *link_url;
	size_t link_len, rewind;

	if (!rndr->make.link)
		return 0;

	link = rndr_newbuf(rndr, BUFFER_SPAN);

	if ((link_len = sd_autolink__www(&rewind, link, data, offset, size)) > 0) {
		link_url = rndr_newbuf(rndr, BUFFER_SPAN);
		BUFPUTSL(link_url, "http://");
		bufput(link_url, link->data, link->size);

		ob->size -= rewind;
		rndr->make.link(ob, link_url, NULL, link, rndr->make.opaque);
		rndr_popbuf(rndr, BUFFER_SPAN);
	}

	rndr_popbuf(rndr, BUFFER_SPAN);
	return link_len;
}

static size_t
char_autolink_email(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size)
{
	struct buf *link;
	size_t link_len, rewind;

	if (!rndr->make.autolink)
		return 0;

	link = rndr_newbuf(rndr, BUFFER_SPAN);

	if ((link_len = sd_autolink__email(&rewind, link, data, offset, size)) > 0) {
		ob->size -= rewind;
		rndr->make.autolink(ob, link, MKDA_EMAIL, rndr->make.opaque);
	}

	rndr_popbuf(rndr, BUFFER_SPAN);
	return link_len;
}

static size_t
char_autolink_url(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size)
{
	struct buf *link;
	size_t link_len, rewind;

	if (!rndr->make.autolink)
		return 0;

	link = rndr_newbuf(rndr, BUFFER_SPAN);

	if ((link_len = sd_autolink__url(&rewind, link, data, offset, size)) > 0) {
		ob->size -= rewind;
		rndr->make.autolink(ob, link, MKDA_NORMAL, rndr->make.opaque);
	}

	rndr_popbuf(rndr, BUFFER_SPAN);
	return link_len;
}

/* char_link • '[': parsing a link or an image */
static size_t
char_link(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size)
{
	int is_img = (offset && data[-1] == '!'), level;
	size_t i = 1, txt_e, link_b = 0, link_e = 0, title_b = 0, title_e = 0;
	struct buf *content = 0;
	struct buf *link = 0;
	struct buf *title = 0;
	struct buf *u_link = 0;
	size_t org_work_size = rndr->work_bufs[BUFFER_SPAN].size;
	int text_has_nl = 0, ret = 0;

	/* checking whether the correct renderer exists */
	if ((is_img && !rndr->make.image) || (!is_img && !rndr->make.link))
		goto cleanup;

	/* looking for the matching closing bracket */
	for (level = 1; i < size; i++) {
		if (data[i] == '\n')
			text_has_nl = 1;

		else if (data[i - 1] == '\\')
			continue;

		else if (data[i] == '[')
			level++;

		else if (data[i] == ']') {
			level--;
			if (level <= 0)
				break;
		}
	}

	if (i >= size)
		goto cleanup;

	txt_e = i;
	i++;

	/* skip any amount of whitespace or newline */
	/* (this is much more laxist than original markdown syntax) */
	while (i < size && isspace(data[i]))
		i++;

	/* inline style link */
	if (i < size && data[i] == '(') {
		/* skipping initial whitespace */
		i++;

		while (i < size && isspace(data[i]))
			i++;

		link_b = i;

		/* looking for link end: ' " ) */
		while (i < size) {
			if (data[i] == '\\') i += 2;
			else if (data[i] == ')' || data[i] == '\'' || data[i] == '"') break;
			else i++;
		}

		if (i >= size) goto cleanup;
		link_e = i;

		/* looking for title end if present */
		if (data[i] == '\'' || data[i] == '"') {
			i++;
			title_b = i;

			while (i < size) {
				if (data[i] == '\\') i += 2;
				else if (data[i] == ')') break;
				else i++;
			}

			if (i >= size) goto cleanup;

			/* skipping whitespaces after title */
			title_e = i - 1;
			while (title_e > title_b && isspace(data[title_e]))
				title_e--;

			/* checking for closing quote presence */
			if (data[title_e] != '\'' &&  data[title_e] != '"') {
				title_b = title_e = 0;
				link_e = i;
			}
		}

		/* remove whitespace at the end of the link */
		while (link_e > link_b && isspace(data[link_e - 1]))
			link_e--;

		/* remove optional angle brackets around the link */
		if (data[link_b] == '<') link_b++;
		if (data[link_e - 1] == '>') link_e--;

		/* building escaped link and title */
		if (link_e > link_b) {
			link = rndr_newbuf(rndr, BUFFER_SPAN);
			bufput(link, data + link_b, link_e - link_b);
		}

		if (title_e > title_b) {
			title = rndr_newbuf(rndr, BUFFER_SPAN);
			bufput(title, data + title_b, title_e - title_b);
		}

		i++;
	}

	/* reference style link */
	else if (i < size && data[i] == '[') {
		struct buf id = { 0, 0, 0, 0, 0 };
		struct link_ref *lr;

		/* looking for the id */
		i++;
		link_b = i;
		while (i < size && data[i] != ']') i++;
		if (i >= size) goto cleanup;
		link_e = i;

		/* finding the link_ref */
		if (link_b == link_e) {
			if (text_has_nl) {
				struct buf *b = rndr_newbuf(rndr, BUFFER_SPAN);
				size_t j;

				for (j = 1; j < txt_e; j++) {
					if (data[j] != '\n')
						bufputc(b, data[j]);
					else if (data[j - 1] != ' ')
						bufputc(b, ' ');
				}

				id.data = b->data;
				id.size = b->size;
			} else {
				id.data = data + 1;
				id.size = txt_e - 1;
			}
		} else {
			id.data = data + link_b;
			id.size = link_e - link_b;
		}

		lr = arr_sorted_find(&rndr->refs, &id, cmp_link_ref);
		if (!lr) goto cleanup;

		/* keeping link and title from link_ref */
		link = lr->link;
		title = lr->title;
		i++;
	}

	/* shortcut reference style link */
	else {
		struct buf id = { 0, 0, 0, 0, 0 };
		struct link_ref *lr;

		/* crafting the id */
		if (text_has_nl) {
			struct buf *b = rndr_newbuf(rndr, BUFFER_SPAN);
			size_t j;

			for (j = 1; j < txt_e; j++) {
				if (data[j] != '\n')
					bufputc(b, data[j]);
				else if (data[j - 1] != ' ')
					bufputc(b, ' ');
			}

			id.data = b->data;
			id.size = b->size;
		} else {
			id.data = data + 1;
			id.size = txt_e - 1;
		}

		/* finding the link_ref */
		lr = arr_sorted_find(&rndr->refs, &id, cmp_link_ref);
		if (!lr) goto cleanup;

		/* keeping link and title from link_ref */
		link = lr->link;
		title = lr->title;

		/* rewinding the whitespace */
		i = txt_e + 1;
	}

	/* building content: img alt is escaped, link content is parsed */
	if (txt_e > 1) {
		content = rndr_newbuf(rndr, BUFFER_SPAN);
		if (is_img) bufput(content, data + 1, txt_e - 1);
		else parse_inline(content, rndr, data + 1, txt_e - 1);
	}

	if (link) {
		u_link = rndr_newbuf(rndr, BUFFER_SPAN);
		unscape_text(u_link, link);
	}

	/* calling the relevant rendering function */
	if (is_img) {
		if (ob->size && ob->data[ob->size - 1] == '!')
			ob->size -= 1;

		ret = rndr->make.image(ob, u_link, title, content, rndr->make.opaque);
	} else {
		ret = rndr->make.link(ob, u_link, title, content, rndr->make.opaque);
	}

	/* cleanup */
cleanup:
	rndr->work_bufs[BUFFER_SPAN].size = (int)org_work_size;
	return ret ? i : 0;
}

static size_t
char_superscript(struct buf *ob, struct render *rndr, char *data, size_t offset, size_t size)
{
	size_t sup_start, sup_len;
	struct buf *sup;

	if (!rndr->make.superscript)
		return 0;

	if (size < 2)
		return 0;

	if (data[1] == '(') {
		sup_start = sup_len = 2;

		while (sup_len < size && data[sup_len] != ')' && data[sup_len - 1] != '\\')
			sup_len++;

		if (sup_len == size)
			return 0;
	} else {
		sup_start = sup_len = 1;

		while (sup_len < size && !isspace(data[sup_len]))
			sup_len++;
	}

	if (sup_len - sup_start == 0)
		return (sup_start == 2) ? 3 : 0;

	sup = rndr_newbuf(rndr, BUFFER_SPAN);
	parse_inline(sup, rndr, data + sup_start, sup_len - sup_start);
	rndr->make.superscript(ob, sup, rndr->make.opaque);
	rndr_popbuf(rndr, BUFFER_SPAN);

	return (sup_start == 2) ? sup_len + 1 : sup_len;
}

/*********************************
 * BLOCK-LEVEL PARSING FUNCTIONS *
 *********************************/

/* is_empty • returns the line length when it is empty, 0 otherwise */
static size_t
is_empty(char *data, size_t size)
{
	size_t i;
	for (i = 0; i < size && data[i] != '\n'; i++)
		if (data[i] != ' ' && data[i] != '\t') return 0;
	return i + 1;
}

/* is_hrule • returns whether a line is a horizontal rule */
static int
is_hrule(char *data, size_t size)
{
	size_t i = 0, n = 0;
	char c;

	/* skipping initial spaces */
	if (size < 3) return 0;
	if (data[0] == ' ') { i++;
	if (data[1] == ' ') { i++;
	if (data[2] == ' ') { i++; } } }

	/* looking at the hrule char */
	if (i + 2 >= size
	|| (data[i] != '*' && data[i] != '-' && data[i] != '_'))
		return 0;
	c = data[i];

	/* the whole line must be the char or whitespace */
	while (i < size && data[i] != '\n') {
		if (data[i] == c) n++;
		else if (data[i] != ' ' && data[i] != '\t')
			return 0;
		i++; }

	return n >= 3;
}

/* check if a line is a code fence; return its size if it is */
static size_t
is_codefence(char *data, size_t size, struct buf *syntax)
{
	size_t i = 0, n = 0;
	char c;

	/* skipping initial spaces */
	if (size < 3) return 0;
	if (data[0] == ' ') { i++;
	if (data[1] == ' ') { i++;
	if (data[2] == ' ') { i++; } } }

	/* looking at the hrule char */
	if (i + 2 >= size || !(data[i] == '~' || data[i] == '`'))
		return 0;

	c = data[i];

	/* the whole line must be the char or whitespace */
	while (i < size && data[i] == c) {
		n++; i++;
	}

	if (n < 3)
		return 0;

	if (syntax != NULL) {
		size_t syn = 0;

		while (i < size && (data[i] == ' ' || data[i] == '\t'))
			i++;

		syntax->data = data + i;

		if (i < size && data[i] == '{') {
			i++; syntax->data++;

			while (i < size && data[i] != '}' && data[i] != '\n') {
				syn++; i++;
			}

			if (i == size || data[i] != '}')
				return 0;

			/* strip all whitespace at the beginning and the end
			 * of the {} block */
			while (syn > 0 && isspace(syntax->data[0])) {
				syntax->data++; syn--;
			}

			while (syn > 0 && isspace(syntax->data[syn - 1]))
				syn--;

			i++;
		} else {
			while (i < size && !isspace(data[i])) {
				syn++; i++;
			}
		}

		syntax->size = syn;
	}

	while (i < size && data[i] != '\n') {
		if (!isspace(data[i]))
			return 0;

		i++;
	}

	return i + 1;
}

/* is_atxheader • returns whether the line is a hash-prefixed header */
static int
is_atxheader(struct render *rndr, char *data, size_t size)
{
	if (data[0] != '#')
		return 0;

	if (rndr->ext_flags & MKDEXT_SPACE_HEADERS) {
		size_t level = 0;

		while (level < size && level < 6 && data[level] == '#')
			level++;

		if (level < size && data[level] != ' ' && data[level] != '\t')
			return 0;
	}

	return 1;
}

/* is_headerline • returns whether the line is a setext-style hdr underline */
static int
is_headerline(char *data, size_t size)
{
	size_t i = 0;

	/* test of level 1 header */
	if (data[i] == '=') {
		for (i = 1; i < size && data[i] == '='; i++);
		while (i < size && (data[i] == ' ' || data[i] == '\t')) i++;
		return (i >= size || data[i] == '\n') ? 1 : 0; }

	/* test of level 2 header */
	if (data[i] == '-') {
		for (i = 1; i < size && data[i] == '-'; i++);
		while (i < size && (data[i] == ' ' || data[i] == '\t')) i++;
		return (i >= size || data[i] == '\n') ? 2 : 0; }

	return 0;
}

/* prefix_quote • returns blockquote prefix length */
static size_t
prefix_quote(char *data, size_t size)
{
	size_t i = 0;
	if (i < size && data[i] == ' ') i++;
	if (i < size && data[i] == ' ') i++;
	if (i < size && data[i] == ' ') i++;
	if (i < size && data[i] == '>') {
		if (i + 1 < size && (data[i + 1] == ' ' || data[i+1] == '\t'))
			return i + 2;
		else return i + 1; }
	else return 0;
}

/* prefix_code • returns prefix length for block code*/
static size_t
prefix_code(char *data, size_t size)
{
	if (size > 0 && data[0] == '\t') return 1;
	if (size > 3 && data[0] == ' ' && data[1] == ' '
			&& data[2] == ' ' && data[3] == ' ') return 4;
	return 0;
}

/* prefix_oli • returns ordered list item prefix */
static size_t
prefix_oli(char *data, size_t size)
{
	size_t i = 0;
	if (i < size && data[i] == ' ') i++;
	if (i < size && data[i] == ' ') i++;
	if (i < size && data[i] == ' ') i++;
	if (i >= size || data[i] < '0' || data[i] > '9') return 0;
	while (i < size && data[i] >= '0' && data[i] <= '9') i++;
	if (i + 1 >= size || data[i] != '.'
	|| (data[i + 1] != ' ' && data[i + 1] != '\t')) return 0;
	return i + 2;
}

/* prefix_uli • returns ordered list item prefix */
static size_t
prefix_uli(char *data, size_t size)
{
	size_t i = 0;
	if (i < size && data[i] == ' ') i++;
	if (i < size && data[i] == ' ') i++;
	if (i < size && data[i] == ' ') i++;
	if (i + 1 >= size
	|| (data[i] != '*' && data[i] != '+' && data[i] != '-')
	|| (data[i + 1] != ' ' && data[i + 1] != '\t'))
		return 0;
	return i + 2;
}


/* parse_block • parsing of one block, returning next char to parse */
static void parse_block(struct buf *ob, struct render *rndr,
			char *data, size_t size);


/* parse_blockquote • handles parsing of a blockquote fragment */
static size_t
parse_blockquote(struct buf *ob, struct render *rndr, char *data, size_t size)
{
	size_t beg, end = 0, pre, work_size = 0;
	char *work_data = 0;
	struct buf *out = 0;

	out = rndr_newbuf(rndr, BUFFER_BLOCK);
	beg = 0;
	while (beg < size) {
		for (end = beg + 1; end < size && data[end - 1] != '\n'; end++);

		pre = prefix_quote(data + beg, end - beg);

		if (pre)
			beg += pre; /* skipping prefix */

		/* empty line followed by non-quote line */
		else if (is_empty(data + beg, end - beg) &&
				(end >= size || (prefix_quote(data + end, size - end) == 0 &&
				!is_empty(data + end, size - end))))
			break;

		if (beg < end) { /* copy into the in-place working buffer */
			/* bufput(work, data + beg, end - beg); */
			if (!work_data)
				work_data = data + beg;
			else if (data + beg != work_data + work_size)
				memmove(work_data + work_size, data + beg, end - beg);
			work_size += end - beg;
		}
		beg = end;
	}

	parse_block(out, rndr, work_data, work_size);
	if (rndr->make.blockquote)
		rndr->make.blockquote(ob, out, rndr->make.opaque);
	rndr_popbuf(rndr, BUFFER_BLOCK);
	return end;
}

static size_t
parse_htmlblock(struct buf *ob, struct render *rndr, char *data, size_t size, int do_render);

/* parse_blockquote • handles parsing of a regular paragraph */
static size_t
parse_paragraph(struct buf *ob, struct render *rndr, char *data, size_t size)
{
	size_t i = 0, end = 0;
	int level = 0;
	struct buf work = { data, 0, 0, 0, 0 }; /* volatile working buffer */

	while (i < size) {
		for (end = i + 1; end < size && data[end - 1] != '\n'; end++) /* empty */;

		if (is_empty(data + i, size - i) || (level = is_headerline(data + i, size - i)) != 0)
			break;

		if (rndr->ext_flags & MKDEXT_LAX_HTML_BLOCKS) {
			if (data[i] == '<' && rndr->make.blockhtml && parse_htmlblock(ob, rndr, data + i, size - i, 0)) {
				end = i;
				break;
			}
		}

		if (is_atxheader(rndr, data + i, size - i) || is_hrule(data + i, size - i)) {
			end = i;
			break;
		}

		i = end;
	}

	work.size = i;
	while (work.size && data[work.size - 1] == '\n')
		work.size--;

	if (!level) {
		struct buf *tmp = rndr_newbuf(rndr, BUFFER_BLOCK);
		parse_inline(tmp, rndr, work.data, work.size);
		if (rndr->make.paragraph)
			rndr->make.paragraph(ob, tmp, rndr->make.opaque);
		rndr_popbuf(rndr, BUFFER_BLOCK);
	} else {
		struct buf *header_work;

		if (work.size) {
			size_t beg;
			i = work.size;
			work.size -= 1;

			while (work.size && data[work.size] != '\n')
				work.size -= 1;

			beg = work.size + 1;
			while (work.size && data[work.size - 1] == '\n')
				work.size -= 1;

			if (work.size > 0) {
				struct buf *tmp = rndr_newbuf(rndr, BUFFER_BLOCK);
				parse_inline(tmp, rndr, work.data, work.size);

				if (rndr->make.paragraph)
					rndr->make.paragraph(ob, tmp, rndr->make.opaque);

				rndr_popbuf(rndr, BUFFER_BLOCK);
				work.data += beg;
				work.size = i - beg;
			}
			else work.size = i;
		}

		header_work = rndr_newbuf(rndr, BUFFER_SPAN);
		parse_inline(header_work, rndr, work.data, work.size);

		if (rndr->make.header)
			rndr->make.header(ob, header_work, (int)level, rndr->make.opaque);

		rndr_popbuf(rndr, BUFFER_SPAN);
	}

	return end;
}

/* parse_fencedcode • handles parsing of a block-level code fragment */
static size_t
parse_fencedcode(struct buf *ob, struct render *rndr, char *data, size_t size)
{
	size_t beg, end;
	struct buf *work = 0;
	struct buf lang = { 0, 0, 0, 0, 0 };

	beg = is_codefence(data, size, &lang);
	if (beg == 0) return 0;

	work = rndr_newbuf(rndr, BUFFER_BLOCK);

	while (beg < size) {
		size_t fence_end;

		fence_end = is_codefence(data + beg, size - beg, NULL);
		if (fence_end != 0) {
			beg += fence_end;
			break;
		}

		for (end = beg + 1; end < size && data[end - 1] != '\n'; end++);

		if (beg < end) {
			/* verbatim copy to the working buffer,
				escaping entities */
			if (is_empty(data + beg, end - beg))
				bufputc(work, '\n');
			else bufput(work, data + beg, end - beg);
		}
		beg = end;
	}

	if (work->size && work->data[work->size - 1] != '\n')
		bufputc(work, '\n');

	if (rndr->make.blockcode)
		rndr->make.blockcode(ob, work, lang.size ? &lang : NULL, rndr->make.opaque);

	rndr_popbuf(rndr, BUFFER_BLOCK);
	return beg;
}

static size_t
parse_blockcode(struct buf *ob, struct render *rndr, char *data, size_t size)
{
	size_t beg, end, pre;
	struct buf *work = 0;

	work = rndr_newbuf(rndr, BUFFER_BLOCK);

	beg = 0;
	while (beg < size) {
		for (end = beg + 1; end < size && data[end - 1] != '\n'; end++) {};
		pre = prefix_code(data + beg, end - beg);

		if (pre)
			beg += pre; /* skipping prefix */
		else if (!is_empty(data + beg, end - beg))
			/* non-empty non-prefixed line breaks the pre */
			break;

		if (beg < end) {
			/* verbatim copy to the working buffer,
				escaping entities */
			if (is_empty(data + beg, end - beg))
				bufputc(work, '\n');
			else bufput(work, data + beg, end - beg);
		}
		beg = end;
	}

	while (work->size && work->data[work->size - 1] == '\n')
		work->size -= 1;

	bufputc(work, '\n');

	if (rndr->make.blockcode)
		rndr->make.blockcode(ob, work, NULL, rndr->make.opaque);

	rndr_popbuf(rndr, BUFFER_BLOCK);
	return beg;
}

/* parse_listitem • parsing of a single list item */
/*	assuming initial prefix is already removed */
static size_t
parse_listitem(struct buf *ob, struct render *rndr, char *data, size_t size, int *flags)
{
	struct buf *work = 0, *inter = 0;
	size_t beg = 0, end, pre, sublist = 0, orgpre = 0, i;
	int in_empty = 0, has_inside_empty = 0;

	/* keeping track of the first indentation prefix */
	while (orgpre < 3 && orgpre < size && data[orgpre] == ' ')
		orgpre++;

	beg = prefix_uli(data, size);
	if (!beg)
		beg = prefix_oli(data, size);

	if (!beg)
		return 0;

	/* skipping to the beginning of the following line */
	end = beg;
	while (end < size && data[end - 1] != '\n')
		end++;

	/* getting working buffers */
	work = rndr_newbuf(rndr, BUFFER_SPAN);
	inter = rndr_newbuf(rndr, BUFFER_SPAN);

	/* putting the first line into the working buffer */
	bufput(work, data + beg, end - beg);
	beg = end;

	/* process the following lines */
	while (beg < size) {
		end++;

		while (end < size && data[end - 1] != '\n')
			end++;

		/* process an empty line */
		if (is_empty(data + beg, end - beg)) {
			in_empty = 1;
			beg = end;
			continue;
		}

		/* calculating the indentation */
		i = 0;
		while (i < 4 && beg + i < end && data[beg + i] == ' ')
			i++;

		pre = i;
		if (data[beg] == '\t') { i = 1; pre = 8; }

		/* checking for a new item */
		if ((prefix_uli(data + beg + i, end - beg - i) &&
			!is_hrule(data + beg + i, end - beg - i)) ||
			prefix_oli(data + beg + i, end - beg - i)) {
			if (in_empty)
				has_inside_empty = 1;

			if (pre == orgpre) /* the following item must have */
				break;             /* the same indentation */

			if (!sublist)
				sublist = work->size;
		}
		/* joining only indented stuff after empty lines */
		else if (in_empty && i < 4 && data[beg] != '\t') {
				*flags |= MKD_LI_END;
				break;
		}
		else if (in_empty) {
			bufputc(work, '\n');
			has_inside_empty = 1;
		}

		in_empty = 0;

		/* adding the line without prefix into the working buffer */
		bufput(work, data + beg + i, end - beg - i);
		beg = end;
	}

	/* render of li contents */
	if (has_inside_empty)
		*flags |= MKD_LI_BLOCK;

	if (*flags & MKD_LI_BLOCK) {
		/* intermediate render of block li */
		if (sublist && sublist < work->size) {
			parse_block(inter, rndr, work->data, sublist);
			parse_block(inter, rndr, work->data + sublist, work->size - sublist); 
		}
		else
			parse_block(inter, rndr, work->data, work->size);
	} else {
		/* intermediate render of inline li */
		if (sublist && sublist < work->size) {
			parse_inline(inter, rndr, work->data, sublist);
			parse_block(inter, rndr, work->data + sublist, work->size - sublist);
		}
		else
			parse_inline(inter, rndr, work->data, work->size);
	}

	/* render of li itself */
	if (rndr->make.listitem)
		rndr->make.listitem(ob, inter, *flags, rndr->make.opaque);

	rndr_popbuf(rndr, BUFFER_SPAN);
	rndr_popbuf(rndr, BUFFER_SPAN);
	return beg;
}


/* parse_list • parsing ordered or unordered list block */
static size_t
parse_list(struct buf *ob, struct render *rndr, char *data, size_t size, int flags)
{
	struct buf *work = 0;
	size_t i = 0, j;

	work = rndr_newbuf(rndr, BUFFER_BLOCK);

	while (i < size) {
		j = parse_listitem(work, rndr, data + i, size - i, &flags);
		i += j;

		if (!j || (flags & MKD_LI_END))
			break;
	}

	if (rndr->make.list)
		rndr->make.list(ob, work, flags, rndr->make.opaque);
	rndr_popbuf(rndr, BUFFER_BLOCK);
	return i;
}

/* parse_atxheader • parsing of atx-style headers */
static size_t
parse_atxheader(struct buf *ob, struct render *rndr, char *data, size_t size)
{
	size_t level = 0;
	size_t i, end, skip;

	while (level < size && level < 6 && data[level] == '#')
		level++;

	for (i = level; i < size && (data[i] == ' ' || data[i] == '\t'); i++);

	for (end = i; end < size && data[end] != '\n'; end++);
	skip = end;

	while (end && data[end - 1] == '#')
		end--;

	while (end && (data[end - 1] == ' ' || data[end - 1] == '\t'))
		end--;

	if (end > i) {
		struct buf *work = rndr_newbuf(rndr, BUFFER_SPAN);

		parse_inline(work, rndr, data + i, end - i);

		if (rndr->make.header)
			rndr->make.header(ob, work, (int)level, rndr->make.opaque);

		rndr_popbuf(rndr, BUFFER_SPAN);
	}

	return skip;
}


/* htmlblock_end • checking end of HTML block : </tag>[ \t]*\n[ \t*]\n */
/*	returns the length on match, 0 otherwise */
static size_t
htmlblock_end(struct html_tag *tag, struct render *rndr, char *data, size_t size)
{
	size_t i, w;

	/* assuming data[0] == '<' && data[1] == '/' already tested */

	/* checking if tag is a match */
	if (tag->size + 3 >= size
	|| strncasecmp(data + 2, tag->text, tag->size)
	|| data[tag->size + 2] != '>')
		return 0;

	/* checking white lines */
	i = tag->size + 3;
	w = 0;
	if (i < size && (w = is_empty(data + i, size - i)) == 0)
		return 0; /* non-blank after tag */
	i += w;
	w = 0;

	if (rndr->ext_flags & MKDEXT_LAX_HTML_BLOCKS) {
		if (i < size)
			w = is_empty(data + i, size - i);
	} else  {
		if (i < size && (w = is_empty(data + i, size - i)) == 0)
			return 0; /* non-blank line after tag line */
	}

	return i + w;
}


/* parse_htmlblock • parsing of inline HTML block */
static size_t
parse_htmlblock(struct buf *ob, struct render *rndr, char *data, size_t size, int do_render)
{
	size_t i, j = 0;
	struct html_tag *curtag;
	int found;
	struct buf work = { data, 0, 0, 0, 0 };

	/* identification of the opening tag */
	if (size < 2 || data[0] != '<') return 0;
	curtag = find_block_tag(data + 1, size - 1);

	/* handling of special cases */
	if (!curtag) {

		/* HTML comment, laxist form */
		if (size > 5 && data[1] == '!' && data[2] == '-' && data[3] == '-') {
			i = 5;

			while (i < size && !(data[i - 2] == '-' && data[i - 1] == '-' && data[i] == '>'))
				i++;

			i++;

			if (i < size)
				j = is_empty(data + i, size - i);

			if (j) {
				work.size = i + j;
				if (do_render && rndr->make.blockhtml)
					rndr->make.blockhtml(ob, &work, rndr->make.opaque);
				return work.size;
			} 
		}

		/* HR, which is the only self-closing block tag considered */
		if (size > 4 && (data[1] == 'h' || data[1] == 'H') && (data[2] == 'r' || data[2] == 'R')) {
			i = 3;
			while (i < size && data[i] != '>')
				i++;

			if (i + 1 < size) {
				i++;
				j = is_empty(data + i, size - i);
				if (j) {
					work.size = i + j;
					if (do_render && rndr->make.blockhtml)
						rndr->make.blockhtml(ob, &work, rndr->make.opaque);
					return work.size;
				}
			} 
		}

		/* no special case recognised */
		return 0;
	}

	/* looking for an unindented matching closing tag */
	/*	followed by a blank line */
	i = 1;
	found = 0;

	/* if not found, trying a second pass looking for indented match */
	/* but not if tag is "ins" or "del" (following original Markdown.pl) */
	if (curtag != INS_TAG && curtag != DEL_TAG) {
		i = 1;
		while (i < size) {
			i++;
			while (i < size && !(data[i - 1] == '<' && data[i] == '/'))
				i++;

			if (i + 2 + curtag->size >= size)
				break;

			j = htmlblock_end(curtag, rndr, data + i - 1, size - i + 1);

			if (j) {
				i += j - 1;
				found = 1;
				break;
			}
		} 
	}

	if (!found) return 0;

	/* the end of the block has been found */
	work.size = i;
	if (do_render && rndr->make.blockhtml)
		rndr->make.blockhtml(ob, &work, rndr->make.opaque);

	return i;
}

static void
parse_table_row(struct buf *ob, struct render *rndr, char *data, size_t size, size_t columns, int *col_data)
{
	size_t i = 0, col;
	struct buf *row_work = 0;

	row_work = rndr_newbuf(rndr, BUFFER_SPAN);

	if (i < size && data[i] == '|')
		i++;

	for (col = 0; col < columns && i < size; ++col) {
		size_t cell_start, cell_end;
		struct buf *cell_work;

		cell_work = rndr_newbuf(rndr, BUFFER_SPAN);

		while (i < size && isspace(data[i]))
			i++;

		cell_start = i;

		while (i < size && data[i] != '|')
			i++;

		cell_end = i - 1;

		while (cell_end > cell_start && isspace(data[cell_end]))
			cell_end--;

		parse_inline(cell_work, rndr, data + cell_start, 1 + cell_end - cell_start);
		if (rndr->make.table_cell)
			rndr->make.table_cell(row_work, cell_work, col_data ? col_data[col] : 0, rndr->make.opaque);

		rndr_popbuf(rndr, BUFFER_SPAN);
		i++;
	}

	for (; col < columns; ++col) {
		struct buf empty_cell = {0, 0, 0, 0, 0};
		if (rndr->make.table_cell)
			rndr->make.table_cell(row_work, &empty_cell, col_data ? col_data[col] : 0, rndr->make.opaque);
	}

	if (rndr->make.table_row)
		rndr->make.table_row(ob, row_work, rndr->make.opaque);

	rndr_popbuf(rndr, BUFFER_SPAN);
}

static size_t
parse_table_header(struct buf *ob, struct render *rndr, char *data, size_t size, size_t *columns, int **column_data)
{
	int pipes;
	size_t i = 0, col, header_end, under_end;

	pipes = 0;
	while (i < size && data[i] != '\n')
		if (data[i++] == '|')
			pipes++;

	if (i == size || pipes == 0)
		return 0;

	header_end = i;

	if (data[0] == '|')
		pipes--;

	if (i > 2 && data[i - 1] == '|')
		pipes--;

	*columns = pipes + 1;
	*column_data = calloc(*columns, sizeof(int));

	/* Parse the header underline */
	i++;
	if (i < size && data[i] == '|')
		i++;

	under_end = i;
	while (under_end < size && data[under_end] != '\n')
		under_end++;

	for (col = 0; col < *columns && i < under_end; ++col) {
		size_t dashes = 0;

		while (i < under_end && (data[i] == ' ' || data[i] == '\t'))
			i++;

		if (data[i] == ':') {
			i++; (*column_data)[col] |= MKD_TABLE_ALIGN_L;
			dashes++;
		}

		while (i < under_end && data[i] == '-') {
			i++; dashes++;
		}

		if (i < under_end && data[i] == ':') {
			i++; (*column_data)[col] |= MKD_TABLE_ALIGN_R;
			dashes++;
		}

		while (i < under_end && (data[i] == ' ' || data[i] == '\t'))
			i++;

		if (i < under_end && data[i] != '|')
			break;

		if (dashes < 3)
			break;

		i++;
	}

	if (col < *columns)
		return 0;

	parse_table_row(ob, rndr, data, header_end, *columns, *column_data);
	return under_end + 1;
}

static size_t
parse_table(struct buf *ob, struct render *rndr, char *data, size_t size)
{
	size_t i;

	struct buf *header_work = 0;
	struct buf *body_work = 0;

	size_t columns;
	int *col_data = NULL;

	header_work = rndr_newbuf(rndr, BUFFER_SPAN);
	body_work = rndr_newbuf(rndr, BUFFER_BLOCK);

	i = parse_table_header(header_work, rndr, data, size, &columns, &col_data);
	if (i > 0) {

		while (i < size) {
			size_t row_start;
			int pipes = 0;

			row_start = i;

			while (i < size && data[i] != '\n')
				if (data[i++] == '|')
					pipes++;

			if (pipes == 0 || i == size) {
				i = row_start;
				break;
			}

			parse_table_row(body_work, rndr, data + row_start, i - row_start, columns, col_data);
			i++;
		}

		if (rndr->make.table)
			rndr->make.table(ob, header_work, body_work, rndr->make.opaque);
	}

	free(col_data);
	rndr_popbuf(rndr, BUFFER_SPAN);
	rndr_popbuf(rndr, BUFFER_BLOCK);
	return i;
}

/* parse_block • parsing of one block, returning next char to parse */
static void
parse_block(struct buf *ob, struct render *rndr, char *data, size_t size)
{
	size_t beg, end, i;
	char *txt_data;
	beg = 0;

	if (rndr->work_bufs[BUFFER_SPAN].size +
		rndr->work_bufs[BUFFER_BLOCK].size > (int)rndr->max_nesting)
		return;

	while (beg < size) {
		txt_data = data + beg;
		end = size - beg;

		if (is_atxheader(rndr, txt_data, end))
			beg += parse_atxheader(ob, rndr, txt_data, end);

		else if (data[beg] == '<' && rndr->make.blockhtml &&
				(i = parse_htmlblock(ob, rndr, txt_data, end, 1)) != 0)
			beg += i;

		else if ((i = is_empty(txt_data, end)) != 0)
			beg += i;

		else if (is_hrule(txt_data, end)) {
			if (rndr->make.hrule)
				rndr->make.hrule(ob, rndr->make.opaque);

			while (beg < size && data[beg] != '\n')
				beg++;

			beg++;
		}

		else if ((rndr->ext_flags & MKDEXT_FENCED_CODE) != 0 &&
			(i = parse_fencedcode(ob, rndr, txt_data, end)) != 0)
			beg += i;

		else if ((rndr->ext_flags & MKDEXT_TABLES) != 0 &&
			(i = parse_table(ob, rndr, txt_data, end)) != 0)
			beg += i;

		else if (prefix_quote(txt_data, end))
			beg += parse_blockquote(ob, rndr, txt_data, end);

		else if (prefix_code(txt_data, end))
			beg += parse_blockcode(ob, rndr, txt_data, end);

		else if (prefix_uli(txt_data, end))
			beg += parse_list(ob, rndr, txt_data, end, 0);

		else if (prefix_oli(txt_data, end))
			beg += parse_list(ob, rndr, txt_data, end, MKD_LIST_ORDERED);

		else
			beg += parse_paragraph(ob, rndr, txt_data, end);
	}
}



/*********************
 * REFERENCE PARSING *
 *********************/

/* is_ref • returns whether a line is a reference or not */
static int
is_ref(char *data, size_t beg, size_t end, size_t *last, struct array *refs)
{
/*	int n; */
	size_t i = 0;
	size_t id_offset, id_end;
	size_t link_offset, link_end;
	size_t title_offset, title_end;
	size_t line_end;
	struct link_ref *lr;
/*	struct buf id = { 0, 0, 0, 0, 0 }; / * volatile buf for id search */

	/* up to 3 optional leading spaces */
	if (beg + 3 >= end) return 0;
	if (data[beg] == ' ') { i = 1;
	if (data[beg + 1] == ' ') { i = 2;
	if (data[beg + 2] == ' ') { i = 3;
	if (data[beg + 3] == ' ') return 0; } } }
	i += beg;

	/* id part: anything but a newline between brackets */
	if (data[i] != '[') return 0;
	i++;
	id_offset = i;
	while (i < end && data[i] != '\n' && data[i] != '\r' && data[i] != ']')
		i++;
	if (i >= end || data[i] != ']') return 0;
	id_end = i;

	/* spacer: colon (space | tab)* newline? (space | tab)* */
	i++;
	if (i >= end || data[i] != ':') return 0;
	i++;
	while (i < end && (data[i] == ' ' || data[i] == '\t')) i++;
	if (i < end && (data[i] == '\n' || data[i] == '\r')) {
		i++;
		if (i < end && data[i] == '\r' && data[i - 1] == '\n') i++; }
	while (i < end && (data[i] == ' ' || data[i] == '\t')) i++;
	if (i >= end) return 0;

	/* link: whitespace-free sequence, optionally between angle brackets */
	if (data[i] == '<') i++;
	link_offset = i;
	while (i < end && data[i] != ' ' && data[i] != '\t'
			&& data[i] != '\n' && data[i] != '\r') i++;
	if (data[i - 1] == '>') link_end = i - 1;
	else link_end = i;

	/* optional spacer: (space | tab)* (newline | '\'' | '"' | '(' ) */
	while (i < end && (data[i] == ' ' || data[i] == '\t')) i++;
	if (i < end && data[i] != '\n' && data[i] != '\r'
			&& data[i] != '\'' && data[i] != '"' && data[i] != '(')
		return 0;
	line_end = 0;
	/* computing end-of-line */
	if (i >= end || data[i] == '\r' || data[i] == '\n') line_end = i;
	if (i + 1 < end && data[i] == '\n' && data[i + 1] == '\r')
		line_end = i + 1;

	/* optional (space|tab)* spacer after a newline */
	if (line_end) {
		i = line_end + 1;
		while (i < end && (data[i] == ' ' || data[i] == '\t')) i++; }

	/* optional title: any non-newline sequence enclosed in '"()
					alone on its line */
	title_offset = title_end = 0;
	if (i + 1 < end
	&& (data[i] == '\'' || data[i] == '"' || data[i] == '(')) {
		i++;
		title_offset = i;
		/* looking for EOL */
		while (i < end && data[i] != '\n' && data[i] != '\r') i++;
		if (i + 1 < end && data[i] == '\n' && data[i + 1] == '\r')
			title_end = i + 1;
		else	title_end = i;
		/* stepping back */
		i -= 1;
		while (i > title_offset && (data[i] == ' ' || data[i] == '\t'))
			i -= 1;
		if (i > title_offset
		&& (data[i] == '\'' || data[i] == '"' || data[i] == ')')) {
			line_end = title_end;
			title_end = i; } }
	if (!line_end) return 0; /* garbage after the link */

	/* a valid ref has been found, filling-in return structures */
	if (last) *last = line_end;
	if (!refs) return 1;
	lr = arr_item(refs, arr_newitem(refs));
	lr->id = bufnew(id_end - id_offset);
	bufput(lr->id, data + id_offset, id_end - id_offset);
	lr->link = bufnew(link_end - link_offset);
	bufput(lr->link, data + link_offset, link_end - link_offset);
	if (title_end > title_offset) {
		lr->title = bufnew(title_end - title_offset);
		bufput(lr->title, data + title_offset,
					title_end - title_offset); }
	else lr->title = 0;
	return 1; 
}

static void expand_tabs(struct buf *ob, const char *line, size_t size)
{
	size_t  i = 0, tab = 0;

	while (i < size) {
		size_t org = i;

		while (i < size && line[i] != '\t') {
			i++; tab++;
		}

		if (i > org)
			bufput(ob, line + org, i - org);

		if (i >= size)
			break;

		do {
			bufputc(ob, ' '); tab++;
		} while (tab % 4);

		i++;
	}
}

/**********************
 * EXPORTED FUNCTIONS *
 **********************/

/* markdown • parses the input buffer and renders it into the output buffer */
void
sd_markdown(struct buf *ob, struct buf *ib, const struct mkd_renderer *rndrer, unsigned int extensions) {
	struct link_ref *lr;
	struct buf *text;
	size_t i, beg, end;
	struct render rndr;

	/* filling the render structure */
	if (!rndrer)
		return;

	text = bufnew(64);
	if (!text)
		return;

	/* Preallocate enough space for our buffer to avoid expanding while copying */
	bufgrow(text, ib->size);

	memcpy(&rndr.make, rndrer, sizeof(struct mkd_renderer));
	arr_init(&rndr.refs, sizeof (struct link_ref));
	parr_init(&rndr.work_bufs[BUFFER_BLOCK]);
	parr_init(&rndr.work_bufs[BUFFER_SPAN]);

	for (i = 0; i < 256; i++)
		rndr.active_char[i] = 0;

	if (rndr.make.emphasis || rndr.make.double_emphasis || rndr.make.triple_emphasis) {
		rndr.active_char['*'] = MD_CHAR_EMPHASIS;
		rndr.active_char['_'] = MD_CHAR_EMPHASIS;
		if (extensions & MKDEXT_STRIKETHROUGH)
			rndr.active_char['~'] = MD_CHAR_EMPHASIS;
	}

	if (rndr.make.codespan)
		rndr.active_char['`'] = MD_CHAR_CODESPAN;

	if (rndr.make.linebreak)
		rndr.active_char['\n'] = MD_CHAR_LINEBREAK;

	if (rndr.make.image || rndr.make.link)
		rndr.active_char['['] = MD_CHAR_LINK;

	rndr.active_char['<'] = MD_CHAR_LANGLE;
	rndr.active_char['\\'] = MD_CHAR_ESCAPE;
	rndr.active_char['&'] = MD_CHAR_ENTITITY;

	if (extensions & MKDEXT_AUTOLINK) {
		rndr.active_char[':'] = MD_CHAR_AUTOLINK_URL;
		rndr.active_char['@'] = MD_CHAR_AUTOLINK_EMAIL;
		rndr.active_char['w'] = MD_CHAR_AUTOLINK_WWW;
	}

	if (extensions & MKDEXT_SUPERSCRIPT)
		rndr.active_char['^'] = MD_CHAR_SUPERSCRIPT;

	/* Extension data */
	rndr.ext_flags = extensions;
	rndr.max_nesting = 16;

	/* first pass: looking for references, copying everything else */
	beg = 0;
	while (beg < ib->size) /* iterating over lines */
		if (is_ref(ib->data, beg, ib->size, &end, &rndr.refs))
			beg = end;
		else { /* skipping to the next line */
			end = beg;
			while (end < ib->size && ib->data[end] != '\n' && ib->data[end] != '\r')
				end++;

			/* adding the line body if present */
			if (end > beg)
				expand_tabs(text, ib->data + beg, end - beg);

			while (end < ib->size && (ib->data[end] == '\n' || ib->data[end] == '\r')) {
				/* add one \n per newline */
				if (ib->data[end] == '\n' || (end + 1 < ib->size && ib->data[end + 1] != '\n'))
					bufputc(text, '\n');
				end++;
			}

			beg = end;
		}

	/* sorting the reference array */
	if (rndr.refs.size)
		qsort(rndr.refs.base, rndr.refs.size, rndr.refs.unit, cmp_link_ref_sort);

	/* second pass: actual rendering */
	if (rndr.make.doc_header)
		rndr.make.doc_header(ob, rndr.make.opaque);

	if (text->size) {
		/* adding a final newline if not already present */
		if (text->data[text->size - 1] != '\n' &&  text->data[text->size - 1] != '\r')
			bufputc(text, '\n');

		parse_block(ob, &rndr, text->data, text->size);
	}

	if (rndr.make.doc_footer)
		rndr.make.doc_footer(ob, rndr.make.opaque);

	/* clean-up */
	bufrelease(text);
	lr = rndr.refs.base;
	for (i = 0; i < (size_t)rndr.refs.size; i++) {
		bufrelease(lr[i].id);
		bufrelease(lr[i].link);
		bufrelease(lr[i].title);
	}

	arr_free(&rndr.refs);

	assert(rndr.work_bufs[BUFFER_SPAN].size == 0);
	assert(rndr.work_bufs[BUFFER_BLOCK].size == 0);

	for (i = 0; i < (size_t)rndr.work_bufs[BUFFER_SPAN].asize; ++i)
		bufrelease(rndr.work_bufs[BUFFER_SPAN].item[i]);

	for (i = 0; i < (size_t)rndr.work_bufs[BUFFER_BLOCK].asize; ++i)
		bufrelease(rndr.work_bufs[BUFFER_BLOCK].item[i]);

	parr_free(&rndr.work_bufs[BUFFER_SPAN]);
	parr_free(&rndr.work_bufs[BUFFER_BLOCK]);
}

void
sd_version(int *ver_major, int *ver_minor, int *ver_revision)
{
	*ver_major = UPSKIRT_VER_MAJOR;
	*ver_minor = UPSKIRT_VER_MINOR;
	*ver_revision = UPSKIRT_VER_REVISION;
}

/* vim: set filetype=c: */
