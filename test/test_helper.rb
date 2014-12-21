# coding: UTF-8
Encoding.default_internal = 'UTF-8' if defined? Encoding

gem 'test-unit', '>= 2' # necessary when not using bundle exec

require 'test/unit'
require 'nokogiri'

require 'redcarpet'
require 'redcarpet/render_strip'
require 'redcarpet/render_man'

class Redcarpet::TestCase < Test::Unit::TestCase
  def html_equal(html_a, html_b)
    assert_equal Nokogiri::HTML::DocumentFragment.parse(html_a).to_html,
      Nokogiri::HTML::DocumentFragment.parse(html_b).to_html
  end

  def assert_renders(html, markdown)
    html_equal html, render(markdown)
  end

  def render(markdown, options = {})
    options = options.fetch(:with, {})

    if options.kind_of?(Array)
      options = Hash[options.map {|o| [o, true]}]
    end

    render = renderer.new(options)
    parser = Redcarpet::Markdown.new(render, options)

    parser.render(markdown)
  end

  private

  def renderer
    @renderer ||= Redcarpet::Render::HTML
  end
end
