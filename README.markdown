Redcarpet 2 is written with sugar, spice and everything nice
============================================================

Redcarpet is Ruby library for Markdown processing that smells like
butterflies and popcorn.

Redcarpet used to be a drop-in replacement for Redcloth. This is no longer the
case since version 2 -- it now has its own API, but retains the old name. Yes,
that does mean that Redcarpet 2 is not backwards-compatible with the 1.X
versions.

Redcarpet is powered by the [Sundown](https://www.github.com/vmg/sundown)
library. You might want to find out more about Sundown to see what makes this
Ruby library so awesome.

This library is written by people
-------------------------------------------------------

Redcarpet 2 has been rewritten from scratch by Vicent Martí (@vmg). Why
are you not following me on Twitter?

Redcarpet would not be possible without the Sundown library and its authors
(Natacha Porté, Vicent Martí, and its many awesome contributors).

You can totally install it as a Gem
-----------------------------------

Redcarpet is readily available as a Ruby gem. It will build some native
extensions, but the parser is standalone and requires no installed libraries.

    $ [sudo] gem install redcarpet

The Redcarpet source (including Sundown as a submodule) is available at GitHub:

    $ git clone git://github.com/vmg/redcarpet.git

And it's like *really* simple to use
------------------------------------

The core of the Redcarpet library is the `Redcarpet::Markdown` class. Each
instance of the class is attached to a `Renderer` object; the Markdown class
performs parsing of a document and uses the attached renderer to generate
output.

The `Markdown` object is encouraged to be instantiated once with the required
settings, and reused between parses.

    Markdown.new(renderer, extensions={})

    Initializes a Markdown parser

    renderer -  a renderer object, inheriting from Redcarpet::Render::Base.
                If the given object has not been instantiated, the library
                will do it with default arguments.

    extensions - a hash containing the Markdown extensions which the parser
                will identify. The following extensions are accepted:

                :no_intra_emphasis - do not parse emphasis inside of words.
                    Strings such as `foo_bar_baz` will not generate `<em>`
                    tags.

                :tables - parse tables, PHP-Markdown style

                :fenced_code_blocks - parse fenced code blocks, PHP-Markdown
                    style. Blocks delimited with 3 or more `~` or backticks
                    will be considered as code, without the need to be
                    indented. An optional language name may be added at the
                    end of the opening fence for the code block

                :autolink - parse links even when they are not enclosed in
                    `<>` characters. Autolinks for the http, https and ftp
                    protocols will be automatically detected. Email addresses
                    are also handled, and http links without protocol, but
                    starting with `www.`

                :strikethrough - parse strikethrough, PHP-Markdown style
                    Two `~` characters mark the start of a strikethrough,
                    e.g. `this is ~~good~~ bad`

                :lax_spacing - HTML blocks do not require to be surrounded
                    by an empty line as in the Markdown standard.

                :space_after_headers - A space is always required between the
                    hash at the beginning of a header and its name, e.g.
                    `#this is my header` would not be a valid header.

                :superscript - parse superscripts after the `^` character;
                    contiguous superscripts are nested together, and complex
                    values can be enclosed in parenthesis,
                    e.g. `this is the 2^(nd) time`

    Example:

        markdown = Redcarpet::Markdown.new(Redcarpet::Render::HTML,
            :autolink => true, :space_after_headers => true)

Rendering with the `Markdown` object is done through `Markdown#render`.
Unlike in the RedCloth API, the text to render is passed as an argument
and not stored inside the `Markdown` instance, to encourage reusability.

    Markdown#render(text)

    Render a Markdown document with the attached renderer

    text - a Markdown document

    Example:

        markdown.render("This is *bongos*, indeed.")
        #=> "<p>This is <em>bongos</em>, indeed</p>"


Darling, I packed you a couple renderers for lunch 
--------------------------------------------------

Redcarpet comes with two built-in renderers, `Redcarpet::Render::HTML` and
`Redcarpet::Render::XHTML`, which output HTML and XHTML, respectively. These
renderers are actually implemented in C, and hence offer a brilliant
performance, several degrees of magnitude faster than other Ruby Markdown
solutions.

All the rendering flags that previously applied only to HTML output have
now been moved to the `Render::HTML` class, and may be enabled when
instantiating the renderer:

    Render::HTML.new(render_options={})

Initializes an HTML renderer. The following flags are available:

    :filter_html - do not allow any user-inputted HTML in the output

    :no_images - do not generate any `<img>` tags

    :no_links - do not generate any `<a>` tags

    :no_styles - do not generate any `<style>` tags

    :safe_links_only - only generate links for protocols which are considered safe

    :with_toc_data - add HTML anchors to each header in the output HTML,
        to allow linking to each section.

    :hard_wrap - insert HTML `<br>` tags inside on paragraphs where the origin
        Markdown document had newlines (by default, Markdown ignores these
        newlines).

    :xhtml - output XHTML-conformant tags. This option is always enabled in the
        `Render::XHTML` renderer.

        :link_attributes - hash of extra attributes to add to links

Example:

    rndr = Redcarpet::Render::HTML.new(:no_links => true, :hard_wrap => true)


The `HTML` renderer has an alternate version, `Redcarpet::Render::HTML_TOC`,
which will output a table of contents in HTML based on the headers of the
Markdown document.

Furthermore, the abstract base class `Redcarpet::Render::Base` can be used
to write a custom renderer purely in Ruby, or extending an existing renderer.
See the following section for more information.


And you can even cook your own
------------------------------

Custom renderers are created by inheriting from an existing renderer. The
built-in renderers, `HTML` and `XHTML` may be extended as such:

~~~~~ ruby
# create a custom renderer that allows highlighting of code blocks
class HTMLwithAlbino < Redcarpet::Render::HTML
  def block_code(code, language)
    Albino.safe_colorize(code, language)
  end
end

markdown = Redcarpet::Markdown.new(HTMLwithAlbino, :fenced_code_blocks => true)
~~~~~

But new renderers can also be created from scratch (see `lib/render_man.rb` for
an example implementation of a Manpage renderer)

~~~~~~ ruby
class ManPage < Redcarpet::Render::Base
    # you get the drill -- keep going from here
end
~~~~~

The following instance methods may be implemented by the renderer:

    # Block-level calls
    # If the return value of the method is `nil`, the block
    # will be skipped.
    # If the method for a document element is not implemented,
    # the block will be skipped.
    # 
    # Example:
    #
    #   class RenderWithoutCode < Redcarpet::Render::HTML
    #     def block_code(code, language)
    #       nil
    #     end
    #   end
    #
    block_code(code, language)
    block_quote(quote)
    block_html(raw_html)
    header(text, header_level)
    hrule()
    list(contents, list_type)
    list_item(text, list_type)
    paragraph(text)
    table(header, body)
    table_row(content)
    table_cell(content, alignment)

    # Span-level calls
    # A return value of `nil` will not output any data
    # If the method for a document element is not implemented,
    # the contents of the span will be copied verbatim
    autolink(link, link_type)
    codespan(code)
    double_emphasis(text)
    emphasis(text)
    image(link, title, alt_text)
    linebreak()
    link(link, title, content)
    raw_html(raw_html)
    triple_emphasis(text)
    strikethrough(text)
    superscript(text)

    # Low level rendering
    entity(text)
    normal_text(text)

    # Header of the document
    # Rendered before any another elements
    doc_header()

    # Footer of the document
    # Rendered after all the other elements
    doc_footer()

    # Pre/post-process
    # Special callback: preprocess or postprocess the whole
    # document before or after the rendering process begins
    preprocess(full_document)
    postprocess(full_document)

You can look at
["How to extend the Redcarpet 2 Markdown library?"](http://dev.af83.com/2012/02/27/howto-extend-the-redcarpet2-markdown-lib.html)
for some more explanations.

Also, now our Pants are much smarter
------------------------------------

Redcarpet 2 comes with a standalone [SmartyPants](
http://daringfireball.net/projects/smartypants/) implementation. It is fully
compliant with the original implementation. It is the fastest SmartyPants
parser there is, with a difference of several orders of magnitude.

The SmartyPants parser can be found in `Redcarpet::Render::SmartyPants`. It has
been implemented as a module, so it can be used standalone or as a mixin.

When mixed with a Renderer class, it will override the `postprocess` method
to perform SmartyPants replacements once the rendering is complete

~~~~ ruby
# Mixin
class HTMLWithPants < Redcarpet::Render::HTML
  include Redcarpet::Render::SmartyPants
end

# Standalone
Redcarpet::Render::SmartyPants.render("<p>Oh SmartyPants, you're so crazy...</p>")
~~~~~

SmartyPants works on top of already-rendered HTML, and will ignore replacements
inside the content of HTML tags and inside specific HTML blocks such as 
`<code>` or `<pre>`.

What? You really want to mix Markdown renderers?
------------------------------------------------

What a terrible idea! Markdown is already ill-specified enough; if you create
software that is renderer-independent, the results will be completely unreliable!

Each renderer has its own API and its own set of extensions: you should choose one
(it doesn't have to be Redcarpet, though that would be great!), write your
software accordingly, and force your users to install it. That's the
only way to have reliable and predictable Markdown output on your program.

Still, if major forces (let's say, tornadoes or other natural disasters) force you
to keep a Markdown-compatibility layer, Redcarpet also supports this:

    require 'redcarpet/compat'

Requiring the compatibility library will declare a `Markdown` class with the
classical RedCloth API, e.g.

    Markdown.new('this is my text').to_html

This class renders 100% standards compliant Markdown with 0 extensions. Nada.
Don't even try to enable extensions with a compatibility layer, because
that's a maintance nightmare and won't work.

On a related topic: if your Markdown gem has a `lib/markdown.rb` file that
monkeypatches the Markdown class, you're a terrible human being. Just saying.

Testing
-------
Tests run a lot faster without `bundle exec` :)

Boring legal stuff
------------------

Copyright (c) 2011, Vicent Martí

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

