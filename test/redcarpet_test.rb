# coding: UTF-8
rootdir = File.dirname(File.dirname(__FILE__))
$LOAD_PATH.unshift "#{rootdir}/lib"

if defined? Encoding
  Encoding.default_internal = 'UTF-8'
end

require 'test/unit'
require 'redcarpet'
require 'redcarpet/render_man'
require 'nokogiri'

def html_equal(html_a, html_b)
  assert_equal Nokogiri::HTML::DocumentFragment.parse(html_a).to_html,
    Nokogiri::HTML::DocumentFragment.parse(html_b).to_html
end

class SmartyPantsTest < Test::Unit::TestCase
  def setup
    @pants = Redcarpet::Render::SmartyPants
  end

  def test_that_smart_converts_single_quotes_in_words_that_end_in_re
    markdown = @pants.render("<p>They're not for sale.</p>")
    assert_equal "<p>They&rsquo;re not for sale.</p>", markdown
  end

  def test_that_smart_converts_single_quotes_in_words_that_end_in_ll
    markdown = @pants.render("<p>Well that'll be the day</p>")
    assert_equal "<p>Well that&rsquo;ll be the day</p>", markdown
  end

  def test_that_smart_converts_double_quotes_to_curly_quotes
    rd = @pants.render(%(<p>"Quoted text"</p>))
    assert_equal %(<p>&ldquo;Quoted text&rdquo;</p>), rd
  end

  def test_that_smart_gives_ve_suffix_a_rsquo
    rd = @pants.render("<p>I've been meaning to tell you ..</p>")
    assert_equal "<p>I&rsquo;ve been meaning to tell you ..</p>", rd
  end

  def test_that_smart_gives_m_suffix_a_rsquo
    rd = @pants.render("<p>I'm not kidding</p>")
    assert_equal "<p>I&rsquo;m not kidding</p>", rd
  end

  def test_that_smart_gives_d_suffix_a_rsquo
    rd = @pants.render("<p>what'd you say?</p>")
    assert_equal "<p>what&rsquo;d you say?</p>", rd
  end
end

class HTMLRenderTest < Test::Unit::TestCase
  def setup
    @markdown = Redcarpet::Markdown.new(Redcarpet::Render::HTML)
    @rndr = {
      :no_html => Redcarpet::Render::HTML.new(:filter_html => true),
      :no_images => Redcarpet::Render::HTML.new(:no_images => true),
      :no_links => Redcarpet::Render::HTML.new(:no_links => true),
      :safe_links => Redcarpet::Render::HTML.new(:safe_links_only => true),
      :escape_html => Redcarpet::Render::HTML.new(:escape_html => true),
      :hard_wrap => Redcarpet::Render::HTML.new(:hard_wrap => true),
    }
  end

  def render_with(rndr, text)
    Redcarpet::Markdown.new(rndr).render(text)
  end

  # Hint: overrides filter_html, no_images and no_links
  def test_that_escape_html_works
    source = <<EOS
Through <em>NO</em> <script>DOUBLE NO</script>

<script>BAD</script>

<img src="/favicon.ico" />
EOS
    expected = <<EOE
<p>Through &lt;em&gt;NO&lt;/em&gt; &lt;script&gt;DOUBLE NO&lt;/script&gt;</p>

<p>&lt;script&gt;BAD&lt;/script&gt;</p>

<p>&lt;img src=&quot;/favicon.ico&quot; /&gt;

EOE

    markdown = render_with(@rndr[:escape_html], source)
    html_equal expected, markdown
  end

  def test_that_filter_html_works
    markdown = render_with(@rndr[:no_html], 'Through <em>NO</em> <script>DOUBLE NO</script>')
    html_equal "<p>Through NO DOUBLE NO</p>", markdown
  end

  def test_filter_html_doesnt_break_two_space_hard_break
    markdown = render_with(@rndr[:no_html], "Lorem,  \nipsum\n")
    html_equal "<p>Lorem,<br/>\nipsum</p>\n", markdown
  end

  def test_that_no_image_flag_works
    rd = render_with(@rndr[:no_images], %(![dust mite](http://dust.mite/image.png) <img src="image.png" />))
    assert rd !~ /<img/
  end

  def test_that_no_links_flag_works
    rd = render_with(@rndr[:no_links], %([This link](http://example.net/) <a href="links.html">links</a>))
    assert rd !~ /<a /
  end

  def test_that_safelink_flag_works
    rd = render_with(@rndr[:safe_links], "[IRC](irc://chat.freenode.org/#freenode)")
    html_equal "<p>[IRC](irc://chat.freenode.org/#freenode)</p>\n", rd
  end

  def test_that_hard_wrap_works
    rd = render_with(@rndr[:hard_wrap], <<EOE)
Hello world,
this is just a simple test

With hard wraps
and other *things*.
EOE
    
    assert rd =~ /<br>/
  end

  def test_that_link_attributes_work
    rndr = Redcarpet::Render::HTML.new(:link_attributes => {:rel => 'blank'})
    md = Redcarpet::Markdown.new(rndr)
    assert md.render('This is a [simple](http://test.com) test.').include?('rel="blank"')
  end
end

class MarkdownTest < Test::Unit::TestCase

  def setup
    @markdown = Redcarpet::Markdown.new(Redcarpet::Render::HTML)
  end

  def render_with(flags, text)
    Redcarpet::Markdown.new(Redcarpet::Render::HTML, flags).render(text)
  end

  def test_that_simple_one_liner_goes_to_html
    assert_respond_to @markdown, :render
    html_equal "<p>Hello World.</p>", @markdown.render("Hello World.")
  end

  def test_that_inline_markdown_goes_to_html
    markdown = @markdown.render('_Hello World_!')
    html_equal "<p><em>Hello World</em>!</p>", markdown
  end

  def test_that_inline_markdown_starts_and_ends_correctly
    markdown = render_with({:no_intra_emphasis => true}, '_start _ foo_bar bar_baz _ end_ *italic* **bold** <a>_blah_</a>')

    html_equal "<p><em>start _ foo_bar bar_baz _ end</em> <em>italic</em> <strong>bold</strong> <a><em>blah</em></a></p>", markdown

    markdown = @markdown.render("Run 'rake radiant:extensions:rbac_base:migrate'")
    html_equal "<p>Run 'rake radiant:extensions:rbac_base:migrate'</p>", markdown
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
    exp = %{<p>Japan: <a href="http://www.abc.net.au/news/events/japan-quake-2011/beforeafter.htm">http://www.abc.net.au/news/events/japan-quake-2011/beforeafter.htm</a> (yes, japan)</p>}
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
    html_equal @markdown.render(<<-header), "<h1>Body</h1>"
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

  def test_strikethrough_flag_works
    text = "this is ~some~ striked ~~text~~"

    assert render_with({}, text) !~ /<del/

    assert render_with({:strikethrough => true}, text) =~ /<del/
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

  def test_that_headers_are_linkable
    markdown = @markdown.render('### Hello [GitHub](http://github.com)')
    html_equal "<h3>Hello <a href=\"http://github.com\">GitHub</a></h3>", markdown
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
    md = Redcarpet::Markdown.new(Redcarpet::Render::HTML, :no_intra_emphasis => true)
    assert render_with({:no_intra_emphasis => true}, "http://en.wikipedia.org/wiki/Dave_Allen_(comedian)") !~ /<em>/
    assert render_with({:no_intra_emphasis => true}, "this fails: hello_world_") !~ /<em>/
    assert render_with({:no_intra_emphasis => true}, "this also fails: hello_world_#bye") !~ /<em>/
    assert render_with({:no_intra_emphasis => true}, "this works: hello_my_world") !~ /<em>/
  end
end

class CustomRenderTest < Test::Unit::TestCase
  class SimpleRender < Redcarpet::Render::HTML
    def emphasis(text)
      "<em class=\"cool\">#{text}</em>"
    end
  end

  def test_simple_overload
    md = Redcarpet::Markdown.new(SimpleRender)
    html_equal "<p>This is <em class=\"cool\">just</em> a test</p>",
      md.render("This is *just* a test")
  end

  class NilPreprocessRenderer < Redcarpet::Render::HTML
    def preprocess(fulldoc)
      nil
    end
  end

  def test_preprocess_returning_nil
    md = Redcarpet::Markdown.new(NilPreprocessRenderer)
    assert_equal(nil,md.render("Anything"))
  end

end

class RedcarpetCompatTest < Test::Unit::TestCase
  def test_simple_compat_api
    html = RedcarpetCompat.new("This is_just_a test").to_html
    html_equal "<p>This is<em>just</em>a test</p>", html
  end
  
  def test_compat_api_enables_extensions
    html = RedcarpetCompat.new("This is_just_a test", :no_intra_emphasis).to_html
    html_equal "<p>This is_just_a test</p>", html
  end

  def test_compat_api_knows_fenced_code_extension
    text = "```ruby\nx = 'foo'\n```"
    html = RedcarpetCompat.new(text, :fenced_code).to_html
    html_equal "<pre><code class=\"ruby\">x = 'foo'\n</code></pre>", html
  end
  
  def test_compat_api_ignores_gh_blockcode_extension
    text = "```ruby\nx = 'foo'\n```"
    html = RedcarpetCompat.new(text, :fenced_code, :gh_blockcode).to_html
    html_equal "<pre><code class=\"ruby\">x = 'foo'\n</code></pre>", html
  end

  def test_compat_api_knows_no_intraemphasis_extension
    html = RedcarpetCompat.new("This is_just_a test", :no_intraemphasis).to_html
    html_equal "<p>This is_just_a test</p>", html
  end
  
  def test_translate_outdated_extensions
    # these extensions are no longer used
    exts = [:gh_blockcode, :no_tables, :smart, :strict]
    html = RedcarpetCompat.new('"TEST"', *exts).to_html
    html_equal "<p>&quot;TEST&quot;</p>", html
  end
end

# Disabled by default
# (these are the easy ones -- the evil ones are not disclosed)
class PathologicalInputsTest # < Test::Unit::TestCase
  def setup
    @markdown = Redcarpet::Markdown.new(Redcarpet::Render::HTML)
  end

  def test_pathological_1
    star = '*'  * 250000
    @markdown.render("#{star}#{star} hi #{star}#{star}")
  end

  def test_pathological_2
    crt = '^' * 255
    str = "#{crt}(\\)"
    @markdown.render("#{str*300}")
  end

  def test_pathological_3
    c = "`t`t`t`t`t`t" * 20000000
    @markdown.render(c)
  end

  def test_pathological_4
    @markdown.render(" [^a]: #{ "A" * 10000 }\n#{ "[^a][]" * 1000000 }\n")
  end

  def test_unbound_recursion
    @markdown.render(("[" * 10000) + "foo" + ("](bar)" * 10000))
  end
end
