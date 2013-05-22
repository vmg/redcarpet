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
Markdown

    html = @markdown.render(markdown)
    html_equal "Foo bar\nHello world! Please visit this site.\nclass Foo\nend\n", html
  end
end
