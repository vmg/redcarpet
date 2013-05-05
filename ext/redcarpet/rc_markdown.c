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

static void rb_redcarpet_md_flags(VALUE hash, unsigned int *enabled_extensions_p)
{
	unsigned int extensions = 0;

	Check_Type(hash, T_HASH);

	/**
	 * Markdown extensions -- all disabled by default
	 */
	if (rb_hash_lookup(hash, CSTR2SYM("no_intra_emphasis")) == Qtrue)
		extensions |= MKDEXT_NO_INTRA_EMPHASIS;

	if (rb_hash_lookup(hash, CSTR2SYM("tables")) == Qtrue)
		extensions |= MKDEXT_TABLES;

	if (rb_hash_lookup(hash, CSTR2SYM("fenced_code_blocks")) == Qtrue)
		extensions |= MKDEXT_FENCED_CODE;

	if (rb_hash_lookup(hash, CSTR2SYM("disable_indented_code_blocks")) == Qtrue)
		extensions |= MKDEXT_DISABLE_INDENTED_CODE;

	if (rb_hash_lookup(hash, CSTR2SYM("autolink")) == Qtrue)
		extensions |= MKDEXT_AUTOLINK;

	if (rb_hash_lookup(hash, CSTR2SYM("strikethrough")) == Qtrue)
		extensions |= MKDEXT_STRIKETHROUGH;

	if (rb_hash_lookup(hash, CSTR2SYM("underline")) == Qtrue)
		extensions |= MKDEXT_UNDERLINE;

	if (rb_hash_lookup(hash, CSTR2SYM("lax_spacing")) == Qtrue)
		extensions |= MKDEXT_LAX_SPACING;

	if (rb_hash_lookup(hash, CSTR2SYM("space_after_headers")) == Qtrue)
		extensions |= MKDEXT_SPACE_HEADERS;

	if (rb_hash_lookup(hash, CSTR2SYM("superscript")) == Qtrue)
		extensions |= MKDEXT_SUPERSCRIPT;

	*enabled_extensions_p = extensions;
}

static void
rb_redcarpet_md__free(void *markdown)
{
	sd_markdown_free((struct sd_markdown *)markdown);
}

static VALUE rb_redcarpet_md__new(int argc, VALUE *argv, VALUE klass)
{
	VALUE rb_markdown, rb_rndr, hash;
	unsigned int extensions = 0;

	struct rb_redcarpet_rndr *rndr;
	struct sd_markdown *markdown;

	if (rb_scan_args(argc, argv, "11", &rb_rndr, &hash) == 2)
		rb_redcarpet_md_flags(hash, &extensions);

	if (rb_obj_is_kind_of(rb_rndr, rb_cClass))
		rb_rndr = rb_funcall(rb_rndr, rb_intern("new"), 0);

	if (!rb_obj_is_kind_of(rb_rndr, rb_cRenderBase))
		rb_raise(rb_eTypeError, "Invalid Renderer instance given");

	Data_Get_Struct(rb_rndr, struct rb_redcarpet_rndr, rndr);

	markdown = sd_markdown_new(extensions, 16, &rndr->callbacks, &rndr->options);
	if (!markdown)
		rb_raise(rb_eRuntimeError, "Failed to create new Renderer class");

	rb_markdown = Data_Wrap_Struct(klass, NULL, rb_redcarpet_md__free, markdown);
	rb_iv_set(rb_markdown, "@renderer", rb_rndr);

	return rb_markdown;
}

static VALUE rb_redcarpet_md_render(VALUE self, VALUE text)
{
	VALUE rb_rndr;
	struct buf *output_buf;
	struct sd_markdown *markdown;

	Check_Type(text, T_STRING);

	rb_rndr = rb_iv_get(self, "@renderer");
	Data_Get_Struct(self, struct sd_markdown, markdown);

	if (rb_respond_to(rb_rndr, rb_intern("preprocess")))
		text = rb_funcall(rb_rndr, rb_intern("preprocess"), 1, text);
  if (NIL_P(text))
    return Qnil;

#ifdef HAVE_RUBY_ENCODING_H
	{
		struct rb_redcarpet_rndr *renderer;
		Data_Get_Struct(rb_rndr, struct rb_redcarpet_rndr, renderer);
		renderer->options.active_enc = rb_enc_get(text);
	}
#endif

	/* initialize buffers */
	output_buf = bufnew(128);

	/* render the magic */
	sd_markdown_render(
		output_buf,
		(const uint8_t*)RSTRING_PTR(text),
		RSTRING_LEN(text),
		markdown);

	/* build the Ruby string */
	text = redcarpet_str_new((const char*)output_buf->data, output_buf->size, rb_enc_get(text));

	bufrelease(output_buf);

	if (rb_respond_to(rb_rndr, rb_intern("postprocess")))
		text = rb_funcall(rb_rndr, rb_intern("postprocess"), 1, text);

	return text;
}

__attribute__((visibility("default")))
void Init_redcarpet()
{
    rb_mRedcarpet = rb_define_module("Redcarpet");

	rb_cMarkdown = rb_define_class_under(rb_mRedcarpet, "Markdown", rb_cObject);
    rb_define_singleton_method(rb_cMarkdown, "new", rb_redcarpet_md__new, -1);
    rb_define_method(rb_cMarkdown, "render", rb_redcarpet_md_render, 1);

	Init_redcarpet_rndr();
}

