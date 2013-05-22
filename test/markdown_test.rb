# coding: UTF-8
require 'test_helper'

class MarkdownTest < Test::Unit::TestCase

  def setup
    @markdown = Redcarpet::Markdown.new(Redcarpet::Render::HTML)
  end

  def render_with(flags, text)
    Redcarpet::Markdown.new(Redcarpet::Render::HTML, flags).render(text)
  end

  def test_that_simple_one_liner_goes_to_html
    assert_respond_to @markdown, :render
    html_equal "<p>Hello World.</p>\n", @markdown.render("Hello World.")
  end

  def test_that_inline_markdown_goes_to_html
    markdown = @markdown.render('_Hello World_!')
    html_equal "<p><em>Hello World</em>!</p>\n", markdown
  end

  def test_that_inline_markdown_starts_and_ends_correctly
    markdown = render_with({:no_intra_emphasis => true}, '_start _ foo_bar bar_baz _ end_ *italic* **bold** <a>_blah_</a>')

    html_equal "<p><em>start _ foo_bar bar_baz _ end</em> <em>italic</em> <strong>bold</strong> <a><em>blah</em></a></p>\n", markdown

    markdown = @markdown.render("Run 'rake radiant:extensions:rbac_base:migrate'")
    html_equal "<p>Run 'rake radiant:extensions:rbac_base:migrate'</p>\n", markdown
  end

  def test_that_urls_are_not_doubly_escaped
    markdown = @markdown.render('[Page 2](/search?query=Markdown+Test&page=2)')
    html_equal "<p><a href=\"/search?query=Markdown+Test&amp;page=2\">Page 2</a></p>\n", markdown
  end

  def test_simple_inline_html
    #markdown = Markdown.new("before\n\n<div>\n  foo\n</div>\nafter")
    markdown = @markdown.render("before\n\n<div>\n  foo\n</div>\n\nafter")
    html_equal "<p>before</p>\n\n<div>\n  foo\n</div>\n\n<p>after</p>\n", markdown
  end

  def test_that_html_blocks_do_not_require_their_own_end_tag_line
    markdown = @markdown.render("Para 1\n\n<div><pre>HTML block\n</pre></div>\n\nPara 2 [Link](#anchor)")
    html_equal "<p>Para 1</p>\n\n<div><pre>HTML block\n</pre></div>\n\n<p>Para 2 <a href=\"#anchor\">Link</a></p>\n",
      markdown
  end

  # This isn't in the spec but is Markdown.pl behavior.
  def test_block_quotes_preceded_by_spaces
    markdown = @markdown.render(
      "A wise man once said:\n\n" +
      " > Isn't it wonderful just to be alive.\n"
    )
    html_equal "<p>A wise man once said:</p>\n\n" +
      "<blockquote><p>Isn't it wonderful just to be alive.</p>\n</blockquote>\n",
      markdown
  end

  def test_para_before_block_html_should_not_wrap_in_p_tag
    markdown = render_with({:lax_spacing => true},
      "Things to watch out for\n" +
      "<ul>\n<li>Blah</li>\n</ul>\n")

    html_equal "<p>Things to watch out for</p>\n\n" +
      "<ul>\n<li>Blah</li>\n</ul>\n", markdown
  end

  # http://github.com/rtomayko/rdiscount/issues/#issue/13
  def test_headings_with_trailing_space
    text = "The Ant-Sugar Tales \n"         +
           "=================== \n\n"        +
           "By Candice Yellowflower   \n"
    html_equal "<h1>The Ant-Sugar Tales </h1>\n\n<p>By Candice Yellowflower   </p>\n", @markdown.render(text)
  end

  def test_that_intra_emphasis_works
    rd = render_with({}, "foo_bar_baz")
    html_equal "<p>foo<em>bar</em>baz</p>\n", rd

    rd = render_with({:no_intra_emphasis => true},"foo_bar_baz")
    html_equal "<p>foo_bar_baz</p>\n", rd
  end

  def test_that_autolink_flag_works
    rd = render_with({:autolink => true}, "http://github.com/rtomayko/rdiscount")
    html_equal "<p><a href=\"http://github.com/rtomayko/rdiscount\">http://github.com/rtomayko/rdiscount</a></p>\n", rd
  end

  if "".respond_to?(:encoding)
    def test_should_return_string_in_same_encoding_as_input
      input = "Yogācāra"
      output = @markdown.render(input)
      assert_equal input.encoding.name, output.encoding.name
    end

    def test_should_return_string_in_same_encoding_not_in_utf8
      input = "testing".encode('US-ASCII')
      output = @markdown.render(input)
      assert_equal input.encoding.name, output.encoding.name
    end

    def test_should_accept_non_utf8_or_ascii
      input = "testing \xAB\xCD".force_encoding('ASCII-8BIT')
      output = @markdown.render(input)
      assert_equal 'ASCII-8BIT', output.encoding.name
    end
  end

  def test_that_tags_can_have_dashes_and_underscores
    rd = @markdown.render("foo <asdf-qwerty>bar</asdf-qwerty> and <a_b>baz</a_b>")
    html_equal "<p>foo <asdf-qwerty>bar</asdf-qwerty> and <a_b>baz</a_b></p>\n", rd
  end

  def test_link_syntax_is_not_processed_within_code_blocks
    markdown = @markdown.render("    This is a code block\n    This is a link [[1]] inside\n")
    html_equal "<pre><code>This is a code block\nThis is a link [[1]] inside\n</code></pre>\n",
      markdown
  end

  def test_whitespace_after_urls
    rd = render_with({:autolink => true}, "Japan: http://www.abc.net.au/news/events/japan-quake-2011/beforeafter.htm (yes, japan)")
    exp = %{<p>Japan: <a href="http://www.abc.net.au/news/events/japan-quake-2011/beforeafter.htm">http://www.abc.net.au/news/events/japan-quake-2011/beforeafter.htm</a> (yes, japan)</p>\n}
    html_equal exp, rd
  end

  def test_memory_leak_when_parsing_char_links
    @markdown.render(<<-leaks)
2. Identify the wild-type cluster and determine all clusters
   containing or contained by it:

       wildtype <- wildtype.cluster(h)
       wildtype.mask <- logical(nclust)
       wildtype.mask[c(contains(h, wildtype),
                       wildtype,
                       contained.by(h, wildtype))] <- TRUE

   This could be more elegant.
    leaks
  end

  def test_infinite_loop_in_header
    html_equal "<h1>Body</h1>\n", @markdown.render(<<-header)
######
#Body#
######
    header
  end

  def test_that_tables_flag_works
    text = <<EOS
 aaa | bbbb
-----|------
hello|sailor
EOS

    assert render_with({}, text) !~ /<table/

    assert render_with({:tables => true}, text) =~ /<table/
  end

  def test_that_tables_work_with_org_table_syntax
    text = <<EOS
| aaa | bbbb |
|-----+------|
|hello|sailor|
EOS

    assert render_with({}, text) !~ /<table/

    assert render_with({:tables => true}, text) =~ /<table/
  end

  def test_strikethrough_flag_works
    text = "this is ~some~ striked ~~text~~"

    assert render_with({}, text) !~ /<del/

    assert render_with({:strikethrough => true}, text) =~ /<del/
  end

  def test_underline_flag_works
    text = "this is *some* text that is _underlined_. ___boom___"

    refute render_with({}, text).include? '<u>underlined</u>'

    output = render_with({:underline => true}, text)
    assert output.include? '<u>underlined</u>'
    assert output.include? '<em>some</em>'
  end

  def test_that_fenced_flag_works
    text = <<fenced
This is a simple test

~~~~~
This is some awesome code
    with tabs and shit
~~~
fenced

    assert render_with({}, text) !~ /<code/

    assert render_with({:fenced_code_blocks => true}, text) =~ /<code/
  end

  def test_that_fenced_flag_works_without_space
    text = "foo\nbar\n```\nsome\ncode\n```\nbaz"
    out = Redcarpet::Markdown.new(Redcarpet::Render::HTML, :fenced_code_blocks => true, :lax_spacing => true).render(text)
    assert out.include?("<pre><code>")

    out = Redcarpet::Markdown.new(Redcarpet::Render::HTML, :fenced_code_blocks => true).render(text)
    assert !out.include?("<pre><code>")
  end

  def test_that_prettify_works
    text = "foo\nbar\n```\nsome\ncode\n```\nbaz"
    out = Redcarpet::Markdown.new(Redcarpet::Render::HTML.new(:prettify => true), :fenced_code_blocks => true).render(text)
    assert !out.include?("<pre><code class=\"prettyprint\">")
  end

  def test_that_indented_flag_works
    text = <<indented
This is a simple text

    This is some awesome code
    with shit

And this is again a simple text
indented

    assert render_with({}, text) =~ /<code/
    assert render_with({:disable_indented_code_blocks => true}, text) !~ /<code/
  end

  def test_that_headers_are_linkable
    markdown = @markdown.render('### Hello [GitHub](http://github.com)')
    html_equal "<h3>Hello <a href=\"http://github.com\">GitHub</a></h3>\n", markdown
  end

  def test_autolinking_with_ent_chars
    markdown = render_with({:autolink => true}, <<text)
This a stupid link: https://github.com/rtomayko/tilt/issues?milestone=1&state=open
text
    html_equal "<p>This a stupid link: <a href=\"https://github.com/rtomayko/tilt/issues?milestone=1&state=open\">https://github.com/rtomayko/tilt/issues?milestone=1&amp;state=open</a></p>\n", markdown
  end

  def test_spaced_headers
    rd = render_with({:space_after_headers => true}, "#123 a header yes\n")
    assert rd !~ /<h1>/
  end

  def test_proper_intra_emphasis
    assert render_with({:no_intra_emphasis => true}, "http://en.wikipedia.org/wiki/Dave_Allen_(comedian)") !~ /<em>/
    assert render_with({:no_intra_emphasis => true}, "this fails: hello_world_") !~ /<em>/
    assert render_with({:no_intra_emphasis => true}, "this also fails: hello_world_#bye") !~ /<em>/
    assert render_with({:no_intra_emphasis => true}, "this works: hello_my_world") !~ /<em>/
  end
end
