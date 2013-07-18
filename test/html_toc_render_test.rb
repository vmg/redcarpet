# coding: UTF-8
require 'test_helper'

class HTMLTOCRenderTest < Test::Unit::TestCase
  def setup
    @render = Redcarpet::Render::HTML_TOC
  end

  def test_simple_toc_render
    markdown = "# A title \n## A subtitle\n## Another on \n### A sub-sub-title"

    renderer = Redcarpet::Markdown.new(@render)
    output = renderer.render(markdown).strip

    assert output.start_with?("<ul>")
    assert output.end_with?("</ul>")

    assert_equal 4, output.split("<ul>").length
    assert_equal 5, output.split("<li>").length
  end
end
