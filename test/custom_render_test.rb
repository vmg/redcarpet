# coding: UTF-8
require 'test_helper'

class CustomRenderTest < Greenmat::TestCase
  class SimpleRender < Greenmat::Render::HTML
    def emphasis(text)
      "<em class=\"cool\">#{text}</em>"
    end
  end

  def test_simple_overload
    md = Greenmat::Markdown.new(SimpleRender)
    assert_equal "<p>This is <em class=\"cool\">just</em> a test</p>\n",
      md.render("This is *just* a test")
  end

  class NilPreprocessRenderer < Greenmat::Render::HTML
    def preprocess(fulldoc)
      nil
    end
  end

  def test_preprocess_returning_nil
    md = Greenmat::Markdown.new(NilPreprocessRenderer)
    assert_equal(nil,md.render("Anything"))
  end

end
