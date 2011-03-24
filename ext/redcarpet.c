#include <stdio.h>
#include "ruby.h"

#include "markdown.h"

static VALUE rb_cRedcarpet;

static void rb_redcarpet__setup_render(VALUE ruby_obj, struct mkd_renderer *rnd)
{
	unsigned int flags = RENDER_EXPAND_TABS;

	/* smart */
	if (rb_funcall(ruby_obj, rb_intern("smart"), 0) == Qtrue)
		flags |= RENDER_SMARTYPANTS;

	/* filter_html */
	if (rb_funcall(ruby_obj, rb_intern("filter_html"), 0) == Qtrue)
		flags |= RENDER_SKIP_HTML;

	/* no_image */
	if (rb_funcall(ruby_obj, rb_intern("no_image"), 0) == Qtrue)
		flags |= RENDER_SKIP_IMAGES;

	/* no_links */
	if (rb_funcall(ruby_obj, rb_intern("no_links"), 0) == Qtrue)
		flags |= RENDER_SKIP_LINKS;

	/* filter_style */
	if (rb_funcall(ruby_obj, rb_intern("filter_styles"), 0) == Qtrue)
		flags |= RENDER_SKIP_STYLE;

	/* autolink */
	if (rb_funcall(ruby_obj, rb_intern("autolink"), 0) == Qtrue)
		flags |= RENDER_AUTOLINK;

	/* safelink */
	if (rb_funcall(ruby_obj, rb_intern("safelink"), 0) == Qtrue)
		flags |= RENDER_SAFELINK;

	init_renderer(rnd, flags);
}

static VALUE rb_redcarpet_to_html(int argc, VALUE *argv, VALUE self)
{
	VALUE text = rb_funcall(self, rb_intern("text"), 0);
	VALUE result;

	struct buf input_buf, *output_buf;
	struct mkd_renderer redcarpet_render;

	Check_Type(text, T_STRING);

	memset(&input_buf, 0x0, sizeof(struct buf));
	input_buf.data = RSTRING_PTR(text);
	input_buf.size = RSTRING_LEN(text);

	output_buf = bufnew(64);

	rb_redcarpet__setup_render(self, &redcarpet_render);
	markdown(output_buf, &input_buf, &redcarpet_render);

	result = rb_str_new(output_buf->data, output_buf->size);
	bufrelease(output_buf);

	/* force the input encoding */
	if (rb_respond_to(text, rb_intern("encoding"))) {
		VALUE encoding = rb_funcall(text, rb_intern("encoding"), 0);
		rb_funcall(result, rb_intern("force_encoding"), 1, encoding);
	}

	return result;
}

void Init_redcarpet()
{
    rb_cRedcarpet = rb_define_class("Redcarpet", rb_cObject);
    rb_define_method(rb_cRedcarpet, "to_html", rb_redcarpet_to_html, -1);
}

