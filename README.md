[![Gem Version](http://img.shields.io/gem/v/greenmat.svg?style=flat)](http://badge.fury.io/rb/greenmat)
[![Build Status](https://travis-ci.org/increments/greenmat.svg?branch=master&style=flat)](https://travis-ci.org/increments/greenmat)

# Greenmat

**Greenmat** is a Ruby library for Markdown processing, based on [Redcarpet](https://github.com/vmg/redcarpet).

It's a core module of [qiita-markdown](https://github.com/increments/qiita-markdown) gem and not intended for direct use. If you are looking for Qiita-specified markdown processor, use qiita-markdown gem.

## Versioning Policy

Greenmat follows Redcarpet's updates by merging the upstream changes.
The version format is `MAJOR.MINOR.PATCH.FORK`.
`MAJOR.MINOR.PATCH` is the same as the version of Redcarpet that Greenmat is based on. `FORK` is incremented on each release of Greenmat itself and reset to zero when any of `MAJOR.MINOR.PATCH` is bumped.

## Acknowledgment

We appreciate Redcarpet project and the contributors for the great efforts!

## Legal

### Redcarpet

Copyright (c) 2011-2014, Vicent Martí

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
