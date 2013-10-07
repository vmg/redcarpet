# coding: UTF-8
require 'test_helper'

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
      :toc_data => Redcarpet::Render::HTML.new(:with_toc_data => true),
      :prettify => Redcarpet::Render::HTML.new(:prettify => true)
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

<p>&lt;img src=&quot;/favicon.ico&quot; /&gt;</p>
EOE

    markdown = render_with(@rndr[:escape_html], source)
    html_equal expected, markdown
  end

  def test_that_filter_html_works
    markdown = render_with(@rndr[:no_html], 'Through <em>NO</em> <script>DOUBLE NO</script>')
    html_equal "<p>Through NO DOUBLE NO</p>\n", markdown
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

  def test_that_link_works_with_quotes
    rd = render_with(Redcarpet::Render::HTML.new, %([This'link"is](http://example.net/)))
    assert_equal "<p><a href=\"http://example.net/\">This&#39;link&quot;is</a></p>\n", rd

    rd = render_with(@rndr[:escape_html], %([This'link"is](http://example.net/)))
    assert_equal "<p><a href=\"http://example.net/\">This&#39;link&quot;is</a></p>\n", rd
  end

  def test_that_code_emphasis_work
    markdown = <<-MD
This should be **`a bold codespan`**
However, this should be *`an emphasised codespan`*

* **`ABC`** or **`DEF`**
* Foo bar
MD

    html = <<HTML
<p>This should be <strong><code>a bold codespan</code></strong>
However, this should be <em><code>an emphasised codespan</code></em></p>

<ul>
<li><strong><code>ABC</code></strong> or <strong><code>DEF</code></strong></li>
<li>Foo bar</li>
</ul>
HTML

    output = render_with(Redcarpet::Render::HTML.new, markdown)
    assert_equal html, output
  end

  def test_that_parenthesis_are_handled_into_links
    markdown = "Hey have a look at the [bash man page](man:bash(1))!"
    html = "<p>Hey have a look at the <a href=\"man:bash(1)\">bash man page</a>!</p>\n"
    output = render_with(Redcarpet::Render::HTML.new, markdown)

    assert_equal html, output
  end

  def test_autolinking_works_as_expected
    markdown = "Example of uri ftp://user:pass@example.com/. Email foo@bar.com and link http://bar.com"
    renderer = Redcarpet::Markdown.new(Redcarpet::Render::HTML, :autolink => true)
    output = renderer.render(markdown)

    assert output.include? '<a href="ftp://user:pass@example.com/">ftp://user:pass@example.com/</a>'
    assert output.include? 'mailto:foo@bar.com'
    assert output.include? '<a href="http://bar.com">'
  end

  def test_that_footnotes_work
    markdown = <<-MD
This is a footnote.[^1]

[^1]: It provides additional information.
MD

    html = <<HTML
<p>This is a footnote.<sup id="fnref1"><a href="#fn1" rel="footnote">1</a></sup></p>

<div class="footnotes">
<hr>
<ol>

<li id="fn1">
<p>It provides additional information.&nbsp;<a href="#fnref1" rev="footnote">&#8617;</a></p>
</li>

</ol>
</div>
HTML

    renderer = Redcarpet::Markdown.new(Redcarpet::Render::HTML, :footnotes => true)
    output = renderer.render(markdown)
    assert_equal html, output
  end

  def test_footnotes_enabled_but_missing_marker
    markdown = <<MD
Some text without a marker

[^1] And a trailing definition
MD
    html = <<HTML
<p>Some text without a marker</p>

<p>[^1] And a trailing definition</p>
HTML

    renderer = Redcarpet::Markdown.new(Redcarpet::Render::HTML, :footnotes => true)
    output = renderer.render(markdown)
    assert_equal html, output
  end

  def test_footnotes_enabled_but_missing_definition
    markdown = "Some text with a marker[^1] but no definition."
    html = "<p>Some text with a marker[^1] but no definition.</p>\n"

    renderer = Redcarpet::Markdown.new(Redcarpet::Render::HTML, :footnotes => true)
    output = renderer.render(markdown)
    assert_equal html, output
  end

  def test_autolink_short_domains
    markdown = "Example of uri ftp://auto/short/domains. Email auto@l.n and link http://a/u/t/o/s/h/o/r/t"
    renderer = Redcarpet::Markdown.new(Redcarpet::Render::HTML, :autolink => true)
    output = renderer.render(markdown)

    assert output.include? '<a href="ftp://auto/short/domains">ftp://auto/short/domains</a>'
    assert output.include? 'mailto:auto@l.n'
    assert output.include? '<a href="http://a/u/t/o/s/h/o/r/t">http://a/u/t/o/s/h/o/r/t</a>'
  end

  def test_toc_heading_id
    markdown = "# First level  heading\n## Second level heading"
    output = render_with(@rndr[:toc_data], markdown)
    assert_match /<h1 id="first-level-heading">/, output
    assert_match /<h2 id="second-level-heading">/, output
  end

  def test_that_prettify_works
    text = <<-Markdown
Foo

~~~ruby
some
code
~~~

Bar
Markdown

    renderer = Redcarpet::Markdown.new(@rndr[:prettify], fenced_code_blocks: true)
    output = renderer.render(text)

    assert output.include?("<code class=\"prettyprint ruby\">")
  end
end
