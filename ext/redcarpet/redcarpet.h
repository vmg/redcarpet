#ifndef REDCARPET_H__
#define REDCARPET_H__

#define RSTRING_NOT_MODIFIED
#include "ruby.h"
#include <stdio.h>

#include <ruby/encoding.h>

#include "markdown.h"
#include "html.h"

#define CSTR2SYM(s) (ID2SYM(rb_intern((s))))

void Init_redcarpet_rndr();

struct redcarpet_renderopt {
	struct html_renderopt html;
	VALUE link_attributes;
	VALUE self;
	VALUE base_class;
	rb_encoding *active_enc;
};

struct rb_redcarpet_rndr {
	struct sd_callbacks callbacks;
	struct redcarpet_renderopt options;
};

#endif
