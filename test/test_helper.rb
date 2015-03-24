# coding: UTF-8
Encoding.default_internal = 'UTF-8' if defined? Encoding

require 'test/unit'

require 'greenmat'
require 'greenmat/render_strip'
require 'greenmat/render_man'

class Greenmat::TestCase < Test::Unit::TestCase
  def assert_renders(html, markdown)
    assert_equal html, render(markdown)
  end

  def render(markdown, options = {})
    options = options.fetch(:with, {})

    if options.kind_of?(Array)
      options = Hash[options.map {|o| [o, true]}]
    end

    render = renderer.new(options)
    parser = Greenmat::Markdown.new(render, options)

    parser.render(markdown)
  end

  private

  def renderer
    @renderer ||= Greenmat::Render::HTML
  end
end
