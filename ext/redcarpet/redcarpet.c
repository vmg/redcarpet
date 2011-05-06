#include <stdio.h>
#include "ruby.h"

#include "markdown.h"
#include "html.h"

typedef enum
{
	REDCARPET_RENDER_HTML,
	REDCARPET_RENDER_TOC
} RendererType;

static VALUE rb_cRedcarpet;

static void rb_redcarpet__get_flags(VALUE ruby_obj,
		unsigned int *enabled_extensions_p,
		unsigned int *render_flags_p)
{
	unsigned int render_flags = HTML_EXPAND_TABS;
	unsigned int extensions = 0;

	/* filter_html */
	if (rb_funcall(ruby_obj, rb_intern("filter_html"), 0) == Qtrue)
		render_flags |= HTML_SKIP_HTML;

	/* no_image */
	if (rb_funcall(ruby_obj, rb_intern("no_image"), 0) == Qtrue)
		render_flags |= HTML_SKIP_IMAGES;

	/* no_links */
	if (rb_funcall(ruby_obj, rb_intern("no_links"), 0) == Qtrue)
		render_flags |= HTML_SKIP_LINKS;

	/* filter_style */
	if (rb_funcall(ruby_obj, rb_intern("filter_styles"), 0) == Qtrue)
		render_flags |= HTML_SKIP_STYLE;

	/* safelink */
	if (rb_funcall(ruby_obj, rb_intern("safelink"), 0) == Qtrue)
		render_flags |= HTML_SAFELINK;

	if (rb_funcall(ruby_obj, rb_intern("generate_toc"), 0) == Qtrue)
		render_flags |= HTML_TOC;

	if (rb_funcall(ruby_obj, rb_intern("hard_wrap"), 0) == Qtrue)
		render_flags |= HTML_HARD_WRAP;

	if (rb_funcall(ruby_obj, rb_intern("gh_blockcode"), 0) == Qtrue)
		render_flags |= HTML_GITHUB_BLOCKCODE;

	if (rb_funcall(ruby_obj, rb_intern("xhtml"), 0) == Qtrue)
		render_flags |= HTML_USE_XHTML;

	/**
	 * Markdown extensions -- all disabled by default 
	 */
	if (rb_funcall(ruby_obj, rb_intern("no_intraemphasis"), 0) == Qtrue)
		extensions |= MKDEXT_NO_INTRA_EMPHASIS;

	if (rb_funcall(ruby_obj, rb_intern("tables"), 0) == Qtrue)
		extensions |= MKDEXT_TABLES;

	if (rb_funcall(ruby_obj, rb_intern("fenced_code"), 0) == Qtrue)
		extensions |= MKDEXT_FENCED_CODE;

	if (rb_funcall(ruby_obj, rb_intern("autolink"), 0) == Qtrue)
		extensions |= MKDEXT_AUTOLINK;

	if (rb_funcall(ruby_obj, rb_intern("strikethrough"), 0) == Qtrue)
		extensions |= MKDEXT_STRIKETHROUGH;

	if (rb_funcall(ruby_obj, rb_intern("lax_htmlblock"), 0) == Qtrue)
		extensions |= MKDEXT_LAX_HTML_BLOCKS;

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
	case REDCARPET_RENDER_HTML:
		upshtml_renderer(&renderer, render_flags);
		break;

	case REDCARPET_RENDER_TOC:
		upshtml_toc_renderer(&renderer);
		break;

	default:
		return Qnil;
	}

	ups_markdown(output_buf, &input_buf, &renderer, enabled_extensions);

	if (rb_funcall(self, rb_intern("smart"), 0) == Qtrue) {
		struct buf *smart_buf = bufnew(128);
		upshtml_smartypants(smart_buf, output_buf);
		result = rb_str_new(smart_buf->data, smart_buf->size);
		bufrelease(smart_buf);
	} else {
		result = rb_str_new(output_buf->data, output_buf->size);
	}

	bufrelease(output_buf);
	upshtml_free_renderer(&renderer);

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
	return rb_redcarpet__render(self, REDCARPET_RENDER_HTML);
}

void Init_redcarpet()
{
    rb_cRedcarpet = rb_define_class("Redcarpet", rb_cObject);
    rb_define_method(rb_cRedcarpet, "to_html", rb_redcarpet_to_html, -1);
    rb_define_method(rb_cRedcarpet, "toc_content", rb_redcarpet_toc, -1);
}

