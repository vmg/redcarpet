# coding: UTF-8
Encoding.default_internal = 'UTF-8' if defined? Encoding

gem 'test-unit', '>= 2' # necessary when not using bundle exec

require 'test/unit'
require 'nokogiri'

require 'redcarpet'
require 'redcarpet/render_strip'
require 'redcarpet/render_man'
require 'redcarpet/compat'

class Redcarpet::TestCase < Test::Unit::TestCase
  def html_equal(html_a, html_b)
    assert_equal Nokogiri::HTML::DocumentFragment.parse(html_a).to_html,
      Nokogiri::HTML::DocumentFragment.parse(html_b).to_html
  end

  def assert_renders(html, markdown)
    html_equal html, parser.render(markdown)
  end

  private

  def parser
    @parser ||= Redcarpet::Markdown.new(Redcarpet::Render::HTML)
  end
end
