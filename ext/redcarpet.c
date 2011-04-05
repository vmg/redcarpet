#include <stdio.h>
#include "ruby.h"

#include "markdown.h"

#define REDCARPET_RECURSION_LIMIT 16

typedef enum
{
	REDCARPET_RENDER_XHTML,
	REDCARPET_RENDER_TOC
} RendererType;

static VALUE rb_cRedcarpet;

static void rb_redcarpet__setup_xhtml(struct mkd_renderer *rnd, VALUE ruby_obj)
{
	unsigned int render_flags = RENDER_EXPAND_TABS;
	unsigned int parser_flags = 0;

	/* smart */
	if (rb_funcall(ruby_obj, rb_intern("smart"), 0) == Qtrue)
		render_flags |= RENDER_SMARTYPANTS;

	/* filter_html */
	if (rb_funcall(ruby_obj, rb_intern("filter_html"), 0) == Qtrue)
		render_flags |= RENDER_SKIP_HTML;

	/* no_image */
	if (rb_funcall(ruby_obj, rb_intern("no_image"), 0) == Qtrue)
		render_flags |= RENDER_SKIP_IMAGES;

	/* no_links */
	if (rb_funcall(ruby_obj, rb_intern("no_links"), 0) == Qtrue)
		render_flags |= RENDER_SKIP_LINKS;

	/* filter_style */
	if (rb_funcall(ruby_obj, rb_intern("filter_styles"), 0) == Qtrue)
		render_flags |= RENDER_SKIP_STYLE;

	/* autolink */
	if (rb_funcall(ruby_obj, rb_intern("autolink"), 0) == Qtrue)
		render_flags |= RENDER_AUTOLINK;

	/* safelink */
	if (rb_funcall(ruby_obj, rb_intern("safelink"), 0) == Qtrue)
		render_flags |= RENDER_SAFELINK;

	if (rb_funcall(ruby_obj, rb_intern("generate_toc"), 0) == Qtrue)
		render_flags |= RENDER_TOC;

	/* parser - strict */
	if (rb_funcall(ruby_obj, rb_intern("strict"), 0) == Qtrue)
		parser_flags |= PARSER_STRICT;

	init_xhtml_renderer(rnd, render_flags, parser_flags, REDCARPET_RECURSION_LIMIT);
}

static VALUE rb_redcarpet__render(VALUE self, RendererType render_type)
{
	VALUE text = rb_funcall(self, rb_intern("text"), 0);
	VALUE result;

	struct buf input_buf, *output_buf;
	struct mkd_renderer renderer;

	Check_Type(text, T_STRING);

	memset(&input_buf, 0x0, sizeof(struct buf));
	input_buf.data = RSTRING_PTR(text);
	input_buf.size = RSTRING_LEN(text);

	output_buf = bufnew(128);
	bufgrow(output_buf, RSTRING_LEN(text) * 1.2f);

	switch (render_type) {
	case REDCARPET_RENDER_XHTML:
		rb_redcarpet__setup_xhtml(&renderer, self);
		break;

	case REDCARPET_RENDER_TOC:
		init_toc_renderer(&renderer, REDCARPET_RECURSION_LIMIT);
		break;

	default:
		return Qnil;
	}

	markdown(output_buf, &input_buf, &renderer);

	result = rb_str_new(output_buf->data, output_buf->size);
	bufrelease(output_buf);
	free_renderer(&renderer);

	/* force the input encoding */
	if (rb_respond_to(text, rb_intern("encoding"))) {
		VALUE encoding = rb_funcall(text, rb_intern("encoding"), 0);
		rb_funcall(result, rb_intern("force_encoding"), 1, encoding);
	}

	return result;
}

static VALUE
rb_redcarpet_toc(int argc, VALUE *argv, VALUE self)
{
	return rb_redcarpet__render(self, REDCARPET_RENDER_TOC);
}

static VALUE
rb_redcarpet_to_html(int argc, VALUE *argv, VALUE self)
{
	return rb_redcarpet__render(self, REDCARPET_RENDER_XHTML);
}

void Init_redcarpet()
{
    rb_cRedcarpet = rb_define_class("Redcarpet", rb_cObject);
    rb_define_method(rb_cRedcarpet, "to_html", rb_redcarpet_to_html, -1);
    rb_define_method(rb_cRedcarpet, "toc_content", rb_redcarpet_toc, -1);
}

