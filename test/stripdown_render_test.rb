# coding: UTF-8
require 'test_helper'

class StripDownRender < Redcarpet::TestCase
  def setup
    @parser = Redcarpet::Markdown.new(Redcarpet::Render::StripDown)
    @parser_tables = Redcarpet::Markdown.new(Redcarpet::Render::StripDown, {tables: true})
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

  def test_tables
    markdown = "| Left-Aligned  | Centre Aligned  | Right Aligned |\n" \
               "| :------------ |:---------------:| -----:|\n" \
               "| col 3 is      | some wordy text | $1600 |\n" \
               "| col 2 is      | centered        |   $12 |"
    expected = "Left-Aligned\tCentre Aligned\tRight Aligned\t\n" \
               "col 3 is\tsome wordy text\t$1600\t\n" \
               "col 2 is\tcentered\t$12\t\n"
    output   = @parser_tables.render(markdown)

    assert_equal expected, output
  end
end
