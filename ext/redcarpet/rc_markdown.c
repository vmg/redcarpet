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
#include "redcarpet.h"

VALUE rb_mRedcarpet;
VALUE rb_cMarkdown;

extern VALUE rb_cRenderBase;

static void rb_redcarpet_md_flags(VALUE ruby_obj, unsigned int *enabled_extensions_p)
{
	unsigned int extensions = 0;

	/**
	 * Markdown extensions -- all disabled by default 
	 */
	if (rb_funcall(ruby_obj, rb_intern("no_intra_emphasis"), 0) == Qtrue)
		extensions |= MKDEXT_NO_INTRA_EMPHASIS;

	if (rb_funcall(ruby_obj, rb_intern("tables"), 0) == Qtrue)
		extensions |= MKDEXT_TABLES;

	if (rb_funcall(ruby_obj, rb_intern("fenced_code_blocks"), 0) == Qtrue)
		extensions |= MKDEXT_FENCED_CODE;

	if (rb_funcall(ruby_obj, rb_intern("autolink"), 0) == Qtrue)
		extensions |= MKDEXT_AUTOLINK;

	if (rb_funcall(ruby_obj, rb_intern("strikethrough"), 0) == Qtrue)
		extensions |= MKDEXT_STRIKETHROUGH;

	if (rb_funcall(ruby_obj, rb_intern("lax_html_blocks"), 0) == Qtrue)
		extensions |= MKDEXT_LAX_HTML_BLOCKS;

	if (rb_funcall(ruby_obj, rb_intern("space_after_headers"), 0) == Qtrue)
		extensions |= MKDEXT_SPACE_HEADERS;

	if (rb_funcall(ruby_obj, rb_intern("superscript"), 0) == Qtrue)
		extensions |= MKDEXT_SUPERSCRIPT;

	*enabled_extensions_p = extensions;
}

static VALUE rb_redcarpet_md_render_with(VALUE self, VALUE rb_rndr, VALUE text)
{
	VALUE result;

	struct rb_redcarpet_rndr *rndr;
	struct buf input_buf, *output_buf;
	unsigned int enabled_extensions = 0;

	Check_Type(text, T_STRING);

	if (!rb_obj_is_kind_of(rb_rndr, rb_cRenderBase))
		rb_raise(rb_eTypeError, "Invalid Renderer instance");

	if (rb_respond_to(rb_rndr, rb_intern("preprocess")))
		text = rb_funcall(rb_rndr, rb_intern("preprocess"), 1, text);

	Data_Get_Struct(rb_rndr, struct rb_redcarpet_rndr, rndr);

	/* initialize buffers */
	memset(&input_buf, 0x0, sizeof(struct buf));
	input_buf.data = RSTRING_PTR(text);
	input_buf.size = RSTRING_LEN(text);

	output_buf = bufnew(128);

	/* render the magic */
	rb_redcarpet_md_flags(self, &enabled_extensions);
	sd_markdown(output_buf, &input_buf, enabled_extensions, &rndr->callbacks, &rndr->options);
	result = rb_str_new(output_buf->data, output_buf->size);
	rb_enc_copy(result, text);

	bufrelease(output_buf);

	if (rb_respond_to(rb_rndr, rb_intern("postprocess")))
		result = rb_funcall(rb_rndr, rb_intern("postprocess"), 1, result);

	return result;
}

static VALUE rb_redcarpet_md_render(VALUE self, VALUE text)
{
	return rb_redcarpet_md_render_with(self, rb_iv_get(self, "@renderer"), text);
}

void Init_redcarpet()
{
    rb_mRedcarpet = rb_define_module("Redcarpet");

	rb_cMarkdown = rb_define_class_under(rb_mRedcarpet, "Markdown", rb_cObject);
    rb_define_method(rb_cMarkdown, "render", rb_redcarpet_md_render, 1);
    rb_define_method(rb_cMarkdown, "render_with", rb_redcarpet_md_render_with, 2);

	Init_redcarpet_rndr();
}

