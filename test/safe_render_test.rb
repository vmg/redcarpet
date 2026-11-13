require 'test_helper'

class SafeRenderTest < Redcarpet::TestCase
  class InlineSafe < Redcarpet::Render::Safe
    def paragraph(text)
      text
    end
  end

  def setup
    @renderer = Redcarpet::Render::Safe
  end

  def test_safe_links_only_is_enabled_by_default
    markdown = "[foo](javascript:alert('foo'))"
    output   = render(markdown)

    assert_not_match %r{a href}, output
  end

  def test_escape_html_is_enabled_by_default
    markdown = "<p>Hello world!</p>"
    output   = render(markdown)

    assert_match %r{&lt;}, output
  end

  def test_html_escaping_in_code_blocks
    markdown = "~~~\n<p>Hello!</p>\n~~~"
    output   = render(markdown)

    assert_match %r{&lt;p&gt;}, output
  end

  def test_lang_class_is_removed
    markdown = "~~~ruby\nclass Foo; end\n~~~"
    output   = render(markdown, with: [:fenced_code_blocks])

    assert_not_match %r{ruby}, output
  end

  def test_paragraph_override_preserves_native_links
    @renderer = InlineSafe

    assert_renders '<a href="https://example.com/">Example</a>',
      '[Example](https://example.com/)'
  end

  def test_paragraph_override_preserves_native_link_validation
    @renderer = InlineSafe

    assert_renders '[Example](javascript:alert(1))',
      '[Example](javascript:alert(1))'
  end

  def test_paragraph_override_preserves_character_references_in_links
    @renderer = InlineSafe

    assert_renders '<a href="https://example.com/?a=1&amp;b=2">A &amp; B</a>',
      '[A &amp; B](https://example.com/?a=1&amp;b=2)'
  end

  def test_paragraph_override_preserves_raw_html_escaping
    @renderer = InlineSafe

    assert_renders '&lt;span&gt;Example&lt;/span&gt;', '<span>Example</span>'
  end
end
