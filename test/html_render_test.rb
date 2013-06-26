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

  def test_that_comments_arent_escaped
    input = "<!-- This is a nice comment! -->"
    output = render_with(@rndr[:escape_html], input)
    assert output.include? input
  end
end
