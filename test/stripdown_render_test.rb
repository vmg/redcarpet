# coding: UTF-8
require 'test_helper'

class StripDownRender < Redcarpet::TestCase
  def setup
    @parser = Redcarpet::Markdown.new(Redcarpet::Render::StripDown)
  end

  def test_titles
    markdown = "# Foo bar"
    output   = @parser.render(markdown)

    assert_equal "Foo bar\n", output
  end

  def test_code_blocks
    markdown = "\tclass Foo\n\tend"
    output   = @parser.render(markdown)

    assert_equal "class Foo\nend\n", output
  end

  def test_images
    markdown = "Look at this ![picture](http://example.org/picture.png)\n" \
               "And this: ![](http://example.org/image.jpg)"
    expected = "Look at this picture http://example.org/picture.png\n" \
               "And this: http://example.org/image.jpg\n"
    output   = @parser.render(markdown)

    assert_equal expected, output
  end

  def test_links
    markdown = "Here's an [example](https://github.com)"
    expected = "Here's an example (https://github.com)\n"
    output   = @parser.render(markdown)

    assert_equal expected, output
  end
end
