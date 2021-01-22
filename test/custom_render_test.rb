# coding: UTF-8
require 'test_helper'

class CustomRenderTest < Greenmat::TestCase
  class SimpleRender < Greenmat::Render::HTML
    def emphasis(text)
      if @options[:no_intra_emphasis]
        return %(<em class="no_intra_emphasis">#{text}</em>)
      end

      %(<em class="cool">#{text}</em>)
    end

    def header(text, level)
      "My little poney" if @options[:with_toc_data]
    end
  end

  def test_simple_overload
    md = Greenmat::Markdown.new(SimpleRender)
    assert_equal "<p>This is <em class=\"cool\">just</em> a test</p>\n",
      md.render("This is *just* a test")
  end

  def test_renderer_options
    parser = Greenmat::Markdown.new(SimpleRender.new(with_toc_data: true))
    output = parser.render("# A title")

    assert_match "My little poney", output
  end

  def test_markdown_options
    parser = Greenmat::Markdown.new(SimpleRender, no_intra_emphasis: true)
    output = parser.render("*foo*")

    assert_match "no_intra_emphasis", output
  end

  def test_original_options_hash_is_not_mutated
    options = { with_toc_data: true }
    render  = SimpleRender.new(options)
    parser  = Greenmat::Markdown.new(render, tables: true)

    computed_options = render.instance_variable_get(:"@options")

    refute_equal computed_options.object_id, options.object_id
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

  def test_base_render_without_quote_callback
    # Regression test for https://github.com/vmg/greenmat/issues/569
    render = Class.new(Greenmat::Render::Base)
    parser = Greenmat::Markdown.new render.new, quote: true

    assert_equal "", parser.render(%(a "quote"))
  end
end
