#ifndef REDCARPET_H__
#define REDCARPET_H__

#define RSTRING_NOT_MODIFIED
#include "ruby.h"
#include <stdio.h>

#ifdef HAVE_RUBY_ENCODING_H
#	include <ruby/encoding.h>
#	define redcarpet_str_new(data, size, enc) rb_enc_str_new(data, size, enc)
#else
#	define redcarpet_str_new(data, size) rb_str_new(data, size)
#endif

#include "markdown.h"
#include "html.h"

#define CSTR2SYM(s) (ID2SYM(rb_intern((s))))

extern void Init_redcarpet_rndr();

struct redcarpet_renderopt {
	struct html_renderopt html;
	VALUE self;
	VALUE base_class;
};

struct rb_redcarpet_rndr {
	struct sd_callbacks callbacks;
	struct redcarpet_renderopt options;
};

#endif
