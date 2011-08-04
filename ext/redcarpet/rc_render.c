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

#define RSTRING_NOT_MODIFIED

#include <ruby.h>
#include "markdown.h"
#include "html.h"

#ifdef HAVE_RUBY_ENCODING_H
#include <ruby/encoding.h>
#else
#define rb_enc_copy(dst, src)
#endif

#define CSTR2SYM(s) (ID2SYM(rb_intern((s))))

#define SPAN_CALLBACK(method_name, ...) {\
	struct redcarpet_pload *load = opaque;\
	VALUE ret = rb_funcall(load->self, rb_intern(method_name), __VA_ARGS__);\
	if (NIL_P(ret)) return 0;\
	Check_Type(ret, T_STRING);\
	bufput(ob, RSTRING_PTR(ret), RSTRING_LEN(ret));\
	return 1;\
}

#define BLOCK_CALLBACK(method_name, ...) {\
	struct redcarpet_pload *load = opaque;\
	VALUE ret = rb_funcall(load->self, rb_intern(method_name), __VA_ARGS__);\
	if (NIL_P(ret)) return;\
	Check_Type(ret, T_STRING);\
	bufput(ob, RSTRING_PTR(ret), RSTRING_LEN(ret));\
}

extern VALUE rb_mRedcarpet;
VALUE rb_mRender;
VALUE rb_cRenderBase;
VALUE rb_cRenderHTML;
VALUE rb_cRenderHTML_TOC;
VALUE rb_mSmartyPants;

struct redcarpet_pload {
	VALUE self;
};

static inline VALUE
buf2str(struct buf *text)
{
	if (!text || !text->size) return Qnil;
	return rb_str_new(text->data, text->size);
}


static void
rndr_blockcode(struct buf *ob, struct buf *text, struct buf *lang, void *opaque)
{
	BLOCK_CALLBACK("block_code", 2, buf2str(text), buf2str(lang));
}

static void
rndr_blockquote(struct buf *ob, struct buf *text, void *opaque)
{
	BLOCK_CALLBACK("block_quote", 1, buf2str(text));
}

static void
rndr_raw_block(struct buf *ob, struct buf *text, void *opaque)
{
	BLOCK_CALLBACK("block_html", 1, buf2str(text));
}

static void
rndr_header(struct buf *ob, struct buf *text, int level, void *opaque)
{
	BLOCK_CALLBACK("header", 2, buf2str(text), INT2FIX(level));
}

static void
rndr_hrule(struct buf *ob, void *opaque)
{
	BLOCK_CALLBACK("hrule", 0);
}

static void
rndr_list(struct buf *ob, struct buf *text, int flags, void *opaque)
{
	BLOCK_CALLBACK("list", 2, buf2str(text),
			(flags & MKD_LIST_ORDERED) ? CSTR2SYM("ordered") : CSTR2SYM("unordered"));
}

static void
rndr_listitem(struct buf *ob, struct buf *text, int flags, void *opaque)
{
	BLOCK_CALLBACK("list_item", 1, buf2str(text));
}

static void
rndr_paragraph(struct buf *ob, struct buf *text, void *opaque)
{
	BLOCK_CALLBACK("paragraph", 1, buf2str(text));
}

static void
rndr_table(struct buf *ob, struct buf *header, struct buf *body, void *opaque)
{
	BLOCK_CALLBACK("table", 2, buf2str(header), buf2str(body));
}

static void
rndr_tablerow(struct buf *ob, struct buf *text, void *opaque)
{
	BLOCK_CALLBACK("table_row", 1, buf2str(text));
}

static void
rndr_tablecell(struct buf *ob, struct buf *text, int align, void *opaque)
{
	VALUE rb_align;

	switch (align) {
	case MKD_TABLE_ALIGN_L:
		rb_align = CSTR2SYM("left");
		break;

	case MKD_TABLE_ALIGN_R:
		rb_align = CSTR2SYM("right");
		break;

	case MKD_TABLE_ALIGN_CENTER:
		rb_align = CSTR2SYM("center");
		break;

	default:
		rb_align = Qnil;
		break;
	}

	BLOCK_CALLBACK("table_cell", 2, buf2str(text), rb_align);
}




/***
 * SPAN LEVEL
 */
static int
rndr_autolink(struct buf *ob, struct buf *link, enum mkd_autolink type, void *opaque)
{
	SPAN_CALLBACK("autolink", 2, buf2str(link),
		type == MKDA_NORMAL ? CSTR2SYM("url") : CSTR2SYM("email"));
}

static int
rndr_codespan(struct buf *ob, struct buf *text, void *opaque)
{
	SPAN_CALLBACK("codespan", 1, buf2str(text));
}

static int
rndr_double_emphasis(struct buf *ob, struct buf *text, void *opaque)
{
	SPAN_CALLBACK("double_emphasis", 1, buf2str(text));
}

static int
rndr_emphasis(struct buf *ob, struct buf *text, void *opaque)
{
	SPAN_CALLBACK("emphasis", 1, buf2str(text));
}

static int
rndr_image(struct buf *ob, struct buf *link, struct buf *title, struct buf *alt, void *opaque)
{
	SPAN_CALLBACK("image", 3, buf2str(link), buf2str(title), buf2str(alt));
}

static int
rndr_linebreak(struct buf *ob, void *opaque)
{
	SPAN_CALLBACK("linebreak", 0);
}

static int
rndr_link(struct buf *ob, struct buf *link, struct buf *title, struct buf *content, void *opaque)
{
	SPAN_CALLBACK("link", 3, buf2str(link), buf2str(title), buf2str(content));
}

static int
rndr_raw_html(struct buf *ob, struct buf *text, void *opaque)
{
	SPAN_CALLBACK("raw_html", 1, buf2str(text));
}

static int
rndr_triple_emphasis(struct buf *ob, struct buf *text, void *opaque)
{
	SPAN_CALLBACK("triple_emphasis", 1, buf2str(text));
}

static int
rndr_strikethrough(struct buf *ob, struct buf *text, void *opaque)
{
	SPAN_CALLBACK("strikethrough", 1, buf2str(text));
}

static int
rndr_superscript(struct buf *ob, struct buf *text, void *opaque)
{
	SPAN_CALLBACK("superscript", 1, buf2str(text));
}

/**
 * direct writes
 */
static void
rndr_entity(struct buf *ob, struct buf *text, void *opaque)
{
	BLOCK_CALLBACK("entity", 1, buf2str(text));
}

static void
rndr_normal_text(struct buf *ob, struct buf *text, void *opaque)
{
	BLOCK_CALLBACK("normal_text", 1, buf2str(text));
}

static void
rndr_doc_header(struct buf *ob, void *opaque)
{
	BLOCK_CALLBACK("doc_header", 0);
}

static void
rndr_doc_footer(struct buf *ob, void *opaque)
{
	BLOCK_CALLBACK("doc_footer", 0);
}

static struct mkd_renderer rb_redcarpet_rndr = {
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
	rndr_superscript,

	rndr_entity,
	rndr_normal_text,

	rndr_doc_header,
	rndr_doc_footer,

	NULL
};

static const char *rb_redcarpet_method_names[] = {
	"block_code",
	"block_quote",
	"block_html",
	"header",
	"hrule",
	"list",
	"list_item",
	"paragraph",
	"table",
	"table_row",
	"table_cell",

	"autolink",
	"codespan",
	"double_emphasis",
	"emphasis",
	"image",
	"linebreak",
	"link",
	"raw_html",
	"triple_emphasis",
	"strikethrough",
	"superscript",

	"entity",
	"normal_text",

	"doc_header",
	"doc_footer"
};

static const size_t rb_redcarpet_method_count = sizeof(rb_redcarpet_method_names)/sizeof(char *);

static void rb_redcarpet_rbase_free(struct mkd_renderer *rndr)
{
	xfree(rndr->opaque);
	xfree(rndr);
}

static VALUE rb_redcarpet_rbase_alloc(VALUE klass)
{
	struct mkd_renderer *rndr = ALLOC(struct mkd_renderer);
	memset(rndr, 0x0, sizeof(struct mkd_renderer));
	return Data_Wrap_Struct(klass, NULL, rb_redcarpet_rbase_free, rndr);
}

static void rb_redcarpet__overload(VALUE self)
{
	size_t i;
	struct mkd_renderer *rndr;

	void **source = (void **)&rb_redcarpet_rndr;
	void **dest;

	Data_Get_Struct(self, struct mkd_renderer, rndr);
	dest = (void **)rndr;

	for (i = 0; i < rb_redcarpet_method_count; ++i) {
		if (rb_respond_to(self, rb_intern(rb_redcarpet_method_names[i])))
			dest[i] = source[i];
	}
}

static VALUE rb_redcarpet_rbase_init(VALUE self)
{
	struct mkd_renderer *rndr;
	struct redcarpet_pload *payload;

	Data_Get_Struct(self, struct mkd_renderer, rndr);
	rb_redcarpet__overload(self);

	payload = ALLOC(struct redcarpet_pload);
	payload->self = self;

	rndr->opaque = payload;
	return Qnil;
}

static VALUE rb_redcarpet_html_init(int argc, VALUE *argv, VALUE self)
{
	struct mkd_renderer *rndr;
	unsigned int render_flags = 0;
	VALUE hash;

	Data_Get_Struct(self, struct mkd_renderer, rndr);

	if (rb_scan_args(argc, argv, "01", &hash) == 1)
	{
		Check_Type(hash, T_HASH);

		/* filter_html */
		if (rb_hash_aref(hash, CSTR2SYM("filter_html")) == Qtrue)
			render_flags |= HTML_SKIP_HTML;

		/* no_image */
		if (rb_hash_aref(hash, CSTR2SYM("no_image")) == Qtrue)
			render_flags |= HTML_SKIP_IMAGES;

		/* no_links */
		if (rb_hash_aref(hash, CSTR2SYM("no_links")) == Qtrue)
			render_flags |= HTML_SKIP_LINKS;

		/* filter_style */
		if (rb_hash_aref(hash, CSTR2SYM("no_styles")) == Qtrue)
			render_flags |= HTML_SKIP_STYLE;

		/* safelink */
		if (rb_hash_aref(hash, CSTR2SYM("safe_links_only")) == Qtrue)
			render_flags |= HTML_SAFELINK;

		if (rb_hash_aref(hash, CSTR2SYM("with_toc_data")) == Qtrue)
			render_flags |= HTML_TOC;

		if (rb_hash_aref(hash, CSTR2SYM("hard_wrap")) == Qtrue)
			render_flags |= HTML_HARD_WRAP;

		if (rb_hash_aref(hash, CSTR2SYM("xhtml")) == Qtrue)
			render_flags |= HTML_USE_XHTML;	
	}

	sdhtml_renderer(rndr, render_flags, (void *)self);

	if (rb_obj_class(self) != rb_cRenderHTML)
		rb_redcarpet__overload(self);

	return Qnil;
}

static VALUE rb_redcarpet_htmltoc_init(VALUE self)
{
	struct mkd_renderer *rndr;
	Data_Get_Struct(self, struct mkd_renderer, rndr);

	sdhtml_toc_renderer(rndr, (void *)self);

	if (rb_obj_class(self) != rb_cRenderHTML_TOC)
		rb_redcarpet__overload(self);

	return Qnil;
}

static VALUE rb_redcarpet_smartypants_render(VALUE self, VALUE text)
{
	VALUE result;
	struct buf input_buf, *output_buf;

	Check_Type(text, T_STRING);

	memset(&input_buf, 0x0, sizeof(struct buf));
	input_buf.data = RSTRING_PTR(text);
	input_buf.size = RSTRING_LEN(text);

	output_buf = bufnew(128);
	bufgrow(output_buf, RSTRING_LEN(text) * 1.1f);

	sdhtml_smartypants(output_buf, &input_buf);
	result = rb_str_new(output_buf->data, output_buf->size);
	rb_enc_copy(result, text);

	bufrelease(output_buf);
	return result;
}


void Init_redcarpet_rndr()
{
	rb_mRender = rb_define_module_under(rb_mRedcarpet, "Render");

	rb_cRenderBase = rb_define_class_under(rb_mRender, "Base", rb_cObject);
	rb_define_alloc_func(rb_cRenderBase, rb_redcarpet_rbase_alloc);
	rb_define_method(rb_cRenderBase, "initialize", rb_redcarpet_rbase_init, 0);

	rb_cRenderHTML = rb_define_class_under(rb_mRender, "HTML", rb_cRenderBase);
	rb_define_method(rb_cRenderHTML, "initialize", rb_redcarpet_html_init, -1);

	rb_cRenderHTML_TOC = rb_define_class_under(rb_mRender, "HTML_TOC", rb_cRenderBase);
	rb_define_method(rb_cRenderHTML_TOC, "initialize", rb_redcarpet_htmltoc_init, 0);

	rb_mSmartyPants = rb_define_module_under(rb_mRender, "SmartyPants");
	rb_define_method(rb_mSmartyPants, "postprocess", rb_redcarpet_smartypants_render, 1);
}
