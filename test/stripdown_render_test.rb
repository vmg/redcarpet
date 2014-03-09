# coding: UTF-8
require 'test_helper'

class StripDownRender < Test::Unit::TestCase

  def setup
    @markdown = Redcarpet::Markdown.new(Redcarpet::Render::StripDown)
  end

  def test_basics
    markdown = <<-Markdown
# [Foo bar](https://github.com)
Markdown
    html = @markdown.render(markdown)
    html_equal "Foo bar\n", html
  end

  def test_insert_new_lines_char
    markdown = <<-Markdown
# Foo bar

Hello world! Please visit [this site](https://github.com/).

    class Foo
    end

Look at this ![picture](http://example.org/picture.png)
And this: ![](http://example.org/image.jpg)
Markdown
    plaintext = <<-Plaintext
Foo bar
Hello world! Please visit this site.
class Foo
end
Look at this picture http://example.org/picture.png
And this: http://example.org/image.jpg
Plaintext

    html = @markdown.render(markdown)
    html_equal plaintext, html
  end
end
