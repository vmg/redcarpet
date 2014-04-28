# coding: UTF-8
require 'test_helper'

class HTMLTOCRenderTest < Test::Unit::TestCase
  def setup
    @render = Redcarpet::Render::HTML_TOC
    @markdown = "# A title \n## A __nice__ subtitle\n## Another one \n### A sub-sub-title"
  end

  def test_simple_toc_render
    renderer = Redcarpet::Markdown.new(@render)
    output = renderer.render(@markdown).strip

    assert output.start_with?("<ul>")
    assert output.end_with?("</ul>")

    assert_equal 4, output.split("<ul>").length
    assert_equal 5, output.split("<li>").length
  end

  def test_granular_toc_render
    renderer = Redcarpet::Markdown.new(@render.new(nesting_level: 2))
    output = renderer.render(@markdown).strip

    assert output.start_with?("<ul>")
    assert output.end_with?("</ul>")

    assert_equal 4, output.split("<li>").length
    assert !output.include?("A sub-sub title")
  end

  def test_toc_heading_id
    renderer = Redcarpet::Markdown.new(@render)
    output = renderer.render(@markdown)

    assert_match /a-title/, output
    assert_match /a-nice-subtitle/, output
    assert_match /another-one/, output
    assert_match /a-sub-sub-title/, output
  end
end
