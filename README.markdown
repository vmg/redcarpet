Ruby + Upskirt = Markdown that doesn't suck
===========================================

Redcarpet is a Ruby wrapper for Upskirt. It is mostly based on Ryan
Tomayko's RDiscount wrapper, and inspired by Rick Astley wearing a kilt.

Redcarpet is powered by the Upskirt library, which can be found at

	https://www.github.com/tanoku/upskirt

You might want to find out more about Upskirt to see what makes these Ruby
bindings so awesome.

Credits
-------

* Natacha Porté, lady of Markdown
* Vicent Martí, wannabe
* With special thanks to Ryan Tomayko

Install
-------

Redcarpet is readily available as a Ruby gem:

    $ [sudo] gem install redcarpet

The Redcarpet source (including Upskirt as a submodule) is available at GitHub:

    $ git clone git://github.com/tanoku/redcarpet.git

Usage
-----

Redcarpet implements the basic protocol popularized by RedCloth:

~~~~~~ {ruby}
require 'redcarpet'
markdown = Redcarpet.new("Hello World!")
puts markdown.to_html
~~~~~~

Additional processing options can be turned on when creating the
Redcarpet object:

~~~~~~ {ruby}
markdown = Redcarpet.new("Hello World!", :smart, :filter_html)
~~~~~~

Note that by default, Redcarpet parses standard Markdown (with no extensions)
and offers a sane subset of parse options which allow you to modify the rendering
output and to enable MD extensions on a per-case basis.

Redcarpet also offers a wrapper class, `RedcarpetCompat` with the same flags
and behavior as the RDiscount library, which acts as a drop-in replacement.

License
-------

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

