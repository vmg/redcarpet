require 'test_helper'

class HTML5Test < Redcarpet::TestCase
  def test_that_html5_works
    section = <<EOS
<section>
  <p>The quick brown fox jumps over the lazy dog.</p>
</section>
EOS

    figure = <<EOS
<figure>
  <img src="http://example.org/image.jpg" alt="">
  <figcaption>
    <p>Hello world!</p>
  </figcaption>
</figure>
EOS

    assert_renders section, section
    assert_renders figure, figure
  end

  def test_that_html5_works_with_code_blocks
    section = <<EOS
\t<section>
\t\t<p>The quick brown fox jumps over the lazy dog.</p>
\t</section>
EOS

    section_expected = <<EOE
<pre><code>&lt;section&gt;
    &lt;p&gt;The quick brown fox jumps over the lazy dog.&lt;/p&gt;
&lt;/section&gt;
</code></pre>
EOE

    header = <<EOS
    <header>
        <hgroup>
            <h1>Section heading</h1>
            <h2>Subhead</h2>
        </hgroup>
    </header>
EOS

    header_expected = <<EOE
<pre><code>&lt;header&gt;
    &lt;hgroup&gt;
        &lt;h1&gt;Section heading&lt;/h1&gt;
        &lt;h2&gt;Subhead&lt;/h2&gt;
    &lt;/hgroup&gt;
&lt;/header&gt;
</code></pre>
EOE

    assert_renders section_expected, section
    assert_renders header_expected, header
  end

  def test_script_tag_recognition
    markdown = <<-Md
<script type="text/javascript">
  alert('Foo!');
</script>
Md
    assert_renders markdown, markdown
  end
end
