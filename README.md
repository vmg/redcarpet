⚠️ **Deprecated**

Note that this repository is now deprecated.
Please use the [qiita-marker](https://github.com/increments/qiita_marker).

---

[![Gem Version](http://img.shields.io/gem/v/greenmat.svg?style=flat)](http://badge.fury.io/rb/greenmat)
[![Build Status](https://travis-ci.org/increments/greenmat.svg?branch=master&style=flat)](https://travis-ci.org/increments/greenmat)

# Greenmat

**Greenmat** is a Ruby library for Markdown processing, based on [Redcarpet](https://github.com/vmg/redcarpet).

It's a core module of [qiita-markdown](https://github.com/increments/qiita-markdown) gem and not intended for direct use. If you are looking for Qiita-specified markdown processor, use qiita-markdown gem.

## Versioning Policy

Greenmat follows Redcarpet's updates by merging the upstream changes.
The version format is `MAJOR.MINOR.PATCH.FORK`.
`MAJOR.MINOR.PATCH` is the same as the version of Redcarpet that Greenmat is based on. `FORK` is incremented on each release of Greenmat itself and reset to zero when any of `MAJOR.MINOR.PATCH` is bumped.

## Development

### Initial setup

Clone the Greenmat repository:

```bash
$ git clone git@github.com:increments/greenmat.git
$ cd greenmat
```

Set up git remote for Redcarpet as `upstream`:

```bash
$ rake greenmat:setup_upstream
```

### Merging upstream changes

Run the following task to merge `upstream/master` branch into the current branch:

```bash
$ rake greenmat:merge_upstream
```

Note that this task does _not_ automatically commit the merge, so you need to commit the changes after checking each diff. Also it forces conflicting hunks to be auto-resolved cleanly by favoring upstream version.

If you want to merge a branch other than `upstream/master`, specify the name as `BRANCH` environment variable:

```bash
$ rake greenmat:merge_upstream BRANCH=branch_name
```

## Acknowledgment

We appreciate Redcarpet project and the contributors for the great efforts!

## Legal

### Redcarpet

Copyright (c) 2011-2016, Vicent Martí

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
