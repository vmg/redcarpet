#include <stdio.h>
#include "ruby.h"

#include "markdown.h"
#include "xhtml.h"

typedef enum
{
	REDCARPET_RENDER_XHTML,
	REDCARPET_RENDER_TOC
} RendererType;

static VALUE rb_cRedcarpet;

static void rb_redcarpet__get_flags(VALUE ruby_obj,
		unsigned int *enabled_extensions_p,
		unsigned int *render_flags_p)
{
	unsigned int render_flags = XHTML_EXPAND_TABS;
	unsigned int extensions = MKDEXT_TABLES;

	/* smart */
	if (rb_funcall(ruby_obj, rb_intern("smart"), 0) == Qtrue)
		render_flags |= XHTML_SMARTYPANTS;

	/* filter_html */
	if (rb_funcall(ruby_obj, rb_intern("filter_html"), 0) == Qtrue)
		render_flags |= XHTML_SKIP_HTML;

	/* no_image */
	if (rb_funcall(ruby_obj, rb_intern("no_image"), 0) == Qtrue)
		render_flags |= XHTML_SKIP_IMAGES;

	/* no_links */
	if (rb_funcall(ruby_obj, rb_intern("no_links"), 0) == Qtrue)
		render_flags |= XHTML_SKIP_LINKS;

	/* filter_style */
	if (rb_funcall(ruby_obj, rb_intern("filter_styles"), 0) == Qtrue)
		render_flags |= XHTML_SKIP_STYLE;

	/* autolink */
	if (rb_funcall(ruby_obj, rb_intern("autolink"), 0) == Qtrue)
		render_flags |= XHTML_AUTOLINK;

	/* safelink */
	if (rb_funcall(ruby_obj, rb_intern("safelink"), 0) == Qtrue)
		render_flags |= XHTML_SAFELINK;

	if (rb_funcall(ruby_obj, rb_intern("generate_toc"), 0) == Qtrue)
		render_flags |= XHTML_TOC;

	if (rb_funcall(ruby_obj, rb_intern("no_tables"), 0) == Qtrue)
		render_flags |= XHTML_SKIP_TABLES;

	/* parser - strict
	 * This is fucking stupid; what the 'strict' flag actually
	 * enforces is laxer emphasis parsing. So we use a properly
	 * named flag internally, even if outside we have retarded
	 * naming because of compat. issues .*/
	if (rb_funcall(ruby_obj, rb_intern("strict"), 0) == Qtrue)
		extensions |= MKDEXT_LAX_EMPHASIS;

	*enabled_extensions_p = extensions;
	*render_flags_p = render_flags;
}

static VALUE rb_redcarpet__render(VALUE self, RendererType render_type)
{
	VALUE text = rb_funcall(self, rb_intern("text"), 0);
	VALUE result;

	struct buf input_buf, *output_buf;
	struct mkd_renderer renderer;
	unsigned int enabled_extensions, render_flags;

	Check_Type(text, T_STRING);

	memset(&input_buf, 0x0, sizeof(struct buf));
	input_buf.data = RSTRING_PTR(text);
	input_buf.size = RSTRING_LEN(text);

	output_buf = bufnew(128);
	bufgrow(output_buf, RSTRING_LEN(text) * 1.2f);

	rb_redcarpet__get_flags(self, &enabled_extensions, &render_flags);

	switch (render_type) {
	case REDCARPET_RENDER_XHTML:
		init_xhtml_renderer(&renderer, render_flags);
		break;

	case REDCARPET_RENDER_TOC:
		init_toc_renderer(&renderer);
		break;

	default:
		return Qnil;
	}

	markdown(output_buf, &input_buf, &renderer, enabled_extensions);

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

