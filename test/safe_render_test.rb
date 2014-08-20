require 'test_helper'

class SafeRenderTest < Redcarpet::TestCase
  def setup
    @render = Redcarpet::Render::Safe
    @parser = Redcarpet::Markdown.new(@render, fenced_code_blocks: true)
  end

  def test_safe_links_only_is_enabled_by_default
    markdown = "[foo](javascript:alert('foo'))"
    output   = @parser.render(markdown)

    assert_not_match %r{a href}, output
  end

  def test_escape_html_is_enabled_by_default
    markdown = "<p>Hello world!</p>"
    output   = @parser.render(markdown)

    assert_match %r{&lt;}, output
  end

  def test_html_escaping_in_code_blocks
    markdown = "~~~\n<p>Hello!</p>\n~~~"
    output   = @parser.render(markdown)

    assert_match %r{&lt;p&gt;}, output
  end

  def test_lang_class_is_removed
    markdown = "~~~ruby\nclass Foo; end\n~~~"
    output   = @parser.render(markdown)

    assert_not_match %r{ruby}, output
  end
end
