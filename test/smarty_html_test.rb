# coding: UTF-8
require 'test_helper'

class SmartyHTMLTest < Test::Unit::TestCase
  def setup
    @smarty_markdown = Redcarpet::Markdown.new(Redcarpet::Render::SmartyHTML)
  end

  def test_that_smartyhtml_converts_single_quotes
    markdown = @smarty_markdown.render("They're not for sale.")
    assert_equal "<p>They&rsquo;re not for sale.</p>\n", markdown
  end

  def test_that_smartyhtml_converts_double_quotes
    rd = @smarty_markdown.render(%("Quoted text"))
    assert_equal %(<p>&ldquo;Quoted text&rdquo;</p>\n), rd
  end

  def test_that_smartyhtml_ignores_pre
    rd = @smarty_markdown.render("    It's a test of \"pre\"\n")
    expected = "It&#39;s a test of &quot;pre&quot;"
    assert rd.include?(expected), "\"#{rd}\" should contain \"#{expected}\""
  end

  def test_that_smartyhtml_ignores_code
    rd = @smarty_markdown.render("`It's a test of \"code\"`\n")
    expected = "It&#39;s a test of &quot;code&quot;"
    assert rd.include?(expected), "\"#{rd}\" should contain \"#{expected}\""
  end
end
