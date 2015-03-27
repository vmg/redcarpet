#ifndef GREENMAT_H__
#define GREENMAT_H__

#define RSTRING_NOT_MODIFIED
#include "ruby.h"
#include <stdio.h>

#include <ruby/encoding.h>

#include "markdown.h"
#include "html.h"

#define CSTR2SYM(s) (ID2SYM(rb_intern((s))))

void Init_greenmat_rndr();

struct greenmat_renderopt {
	struct html_renderopt html;
	VALUE link_attributes;
	VALUE self;
	VALUE base_class;
	rb_encoding *active_enc;
};

struct rb_greenmat_rndr {
	struct sd_callbacks callbacks;
	struct greenmat_renderopt options;
};

#endif
