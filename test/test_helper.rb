# coding: UTF-8
Encoding.default_internal = 'UTF-8' if defined? Encoding

gem 'test-unit', '>= 2' # necessary when not using bundle exec

require 'test/unit'
require 'nokogiri'

require 'greenmat'
require 'greenmat/render_strip'
require 'greenmat/render_man'
require 'greenmat/compat'

class Greenmat::TestCase < Test::Unit::TestCase
  def html_equal(html_a, html_b)
    assert_equal Nokogiri::HTML::DocumentFragment.parse(html_a).to_html,
      Nokogiri::HTML::DocumentFragment.parse(html_b).to_html
  end

  def assert_renders(html, markdown)
    html_equal html, parser.render(markdown)
  end

  private

  def parser
    @parser ||= Greenmat::Markdown.new(Greenmat::Render::HTML)
  end
end
