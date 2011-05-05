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

#include "buffer.h"
#include "xhtml.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

struct smartypants_data {
	int in_squote;
	int in_dquote;
};

static size_t smartypants_cb__ltag(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size);
static size_t smartypants_cb__dquote(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size);
static size_t smartypants_cb__amp(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size);
static size_t smartypants_cb__period(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size);
static size_t smartypants_cb__number(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size);
static size_t smartypants_cb__dash(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size);
static size_t smartypants_cb__parens(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size);
static size_t smartypants_cb__squote(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size);
static size_t smartypants_cb__backtick(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size);

static size_t (*smartypants_cb_ptrs[])
	(struct buf *, struct smartypants_data *, char, const char *, size_t) = 
{
	NULL,					/* 0 */
	smartypants_cb__dash,	/* 1 */
	smartypants_cb__parens,	/* 2 */
	smartypants_cb__squote, /* 3 */
	smartypants_cb__dquote, /* 4 */
	smartypants_cb__amp,	/* 5 */
	smartypants_cb__period,	/* 6 */
	smartypants_cb__number,	/* 7 */
	smartypants_cb__ltag,	/* 8 */
	smartypants_cb__backtick, /* 9 */
};

static const char smartypants_cb_chars[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 4, 0, 0, 0, 5, 3, 2, 0, 0, 0, 0, 1, 6, 0,
	0, 7, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static inline int
word_boundary(char c)
{
	return c == 0 || isspace(c) || ispunct(c);
}

static int
smartypants_quotes(struct buf *ob, char previous_char, char next_char, char quote, int *is_open)
{
	char ent[8];

	if (*is_open && !word_boundary(next_char))
		return 0;

	if (!(*is_open) && !word_boundary(previous_char))
		return 0;

	snprintf(ent, sizeof(ent), "&%c%cquo;", (*is_open) ? 'r' : 'l', quote);
	*is_open = !(*is_open);
	bufputs(ob, ent);
	return 1;
}

static size_t
smartypants_cb__squote(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size)
{
	if (size >= 2) {
		char t1 = tolower(text[1]);

		if (t1 == '\'') {
			if (smartypants_quotes(ob, previous_char, size >= 3 ? text[2] : 0, 'd', &smrt->in_dquote))
				return 1;
		}

		if ((t1 == 's' || t1 == 't' || t1 == 'm' || t1 == 'd') &&
			(size == 3 || word_boundary(text[2]))) {
			BUFPUTSL(ob, "&rsquo;");
			return 0;
		}

		if (size >= 3) {
			char t2 = tolower(text[2]);

			if (((t1 == 'r' && t2 == 'e') ||
				(t1 == 'l' && t2 == 'l') ||
				(t1 == 'v' && t2 == 'e')) &&
				(size == 4 || word_boundary(text[3]))) {
				BUFPUTSL(ob, "&rsquo;");
				return 0;
			}
		}
	}

	if (smartypants_quotes(ob, previous_char, size > 0 ? text[1] : 0, 's', &smrt->in_squote))
		return 0;

	bufputc(ob, text[0]);
	return 0;
}

static size_t
smartypants_cb__parens(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size)
{
	if (size >= 3) {
		char t1 = tolower(text[1]);
		char t2 = tolower(text[2]);

		if (t1 == 'c' && t2 == ')') {
			BUFPUTSL(ob, "&copy;");
			return 2;
		}

		if (t1 == 'r' && t2 == ')') {
			BUFPUTSL(ob, "&reg;");
			return 2;
		}

		if (size >= 4 && t1 == 't' && t2 == 'm' && text[3] == ')') {
			BUFPUTSL(ob, "&trade;");
			return 3;
		}
	}
	
	bufputc(ob, text[0]);
	return 0;
}

static size_t
smartypants_cb__dash(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size)
{
	if (size >= 2) {
		if (text[1] == '-') {
			BUFPUTSL(ob, "&mdash;");
			return 1;
		}

		if (word_boundary(previous_char) && word_boundary(text[1])) {
			BUFPUTSL(ob, "&ndash;");
			return 0;
		}
	}

	bufputc(ob, text[0]);
	return 0;
}

static size_t
smartypants_cb__amp(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size)
{
	if (size >= 6 && memcmp(text, "&quot;", 6) == 0) {
		if (smartypants_quotes(ob, previous_char, size >= 7 ? text[6] : 0, 'd', &smrt->in_dquote))
			return 5;
	}

	if (size >= 4 && memcmp(text, "&#0;", 4) == 0)
		return 3;

	bufputc(ob, '&');
	return 0;
}

static size_t
smartypants_cb__period(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size)
{
	if (size >= 3 && text[1] == '.' && text[2] == '.') {
		BUFPUTSL(ob, "&hellip;");
		return 2;
	}

	if (size >= 5 && text[1] == ' ' && text[2] == '.' && text[3] == ' ' && text[4] == '.') {
		BUFPUTSL(ob, "&hellip;");
		return 4;
	}

	bufputc(ob, text[0]);
	return 0;
}

static size_t
smartypants_cb__backtick(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size)
{
	if (size >= 2 && text[1] == '`') {
		if (smartypants_quotes(ob, previous_char, size >= 3 ? text[2] : 0, 'd', &smrt->in_dquote))
			return 1;
	}

	return 0;
}

static size_t
smartypants_cb__number(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size)
{
	if (word_boundary(previous_char) && size >= 3) {
		if (text[0] == '1' && text[1] == '/' && text[2] == '2') {
			if (size == 3 || word_boundary(text[3])) {
				BUFPUTSL(ob, "&frac12;");
				return 2;
			}
		}

		if (text[0] == '1' && text[1] == '/' && text[2] == '4') {
			if (size == 3 || word_boundary(text[3]) ||
				(size >= 5 && tolower(text[3]) == 't' && tolower(text[4]) == 'h')) {
				BUFPUTSL(ob, "&frac14;");
				return 2;
			}
		}

		if (text[0] == '3' && text[1] == '/' && text[2] == '4') {
			if (size == 3 || word_boundary(text[3]) ||
				(size >= 6 && tolower(text[3]) == 't' && tolower(text[4]) == 'h' && tolower(text[5]) == 's')) {
				BUFPUTSL(ob, "&frac34;");
				return 2;
			}
		}
	}

	bufputc(ob, text[0]);
	return 0;
}

static size_t
smartypants_cb__dquote(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size)
{
	if (!smartypants_quotes(ob, previous_char, size > 0 ? text[1] : 0, 'd', &smrt->in_dquote))
		BUFPUTSL(ob, "&quot;");

	return 0;
}

static size_t
smartypants_cb__ltag(struct buf *ob, struct smartypants_data *smrt, char previous_char, const char *text, size_t size)
{
	size_t i = 0;

	while (i < size && text[i] != '>')
		i++;

	bufput(ob, text, i + 1);
	return i;
}

#if 0
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
#endif

void
ups_xhtml_smartypants(struct buf *ob, struct buf *text)
{
	size_t i;
	struct smartypants_data smrt = {0, 0};

	if (!text)
		return;

	bufgrow(ob, text->size);

	for (i = 0; i < text->size; ++i) {
		size_t org;
		char action = 0;

		org = i;
		while (i < text->size && (action = smartypants_cb_chars[(unsigned char)text->data[i]]) == 0)
			i++;

		if (i > org)
			bufput(ob, text->data + org, i - org);

		if (i < text->size) {
			i += smartypants_cb_ptrs[(int)action]
				(ob, &smrt, i ? text->data[i - 1] : 0, text->data + i, text->size - i);
		}
	}
}


