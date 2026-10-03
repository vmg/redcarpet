# coding: UTF-8
require 'test_helper'

class ConcurrentRenderTest < Redcarpet::TestCase
  class SlowRender < Redcarpet::Render::Base
    def paragraph(text)
      sleep 0.01
      text
    end
  end

  def test_concurrent_render
    markdown = Redcarpet::Markdown.new(SlowRender)
    threads = (1..5).map do
      Thread.new do
        5.times do
          assert_equal 'hi', markdown.render('hi')
        end
      end
    end
    threads.each(&:join)
  end

  class BadRenderException < StandardError; end

  class ExceptionRender < Redcarpet::Render::Base
    def paragraph(text)
      raise BadRenderException
    end
  end

  def test_lock_released_on_exception
    markdown = Redcarpet::Markdown.new(ExceptionRender)
    assert_raises BadRenderException do
      markdown.render('hi')
    end
    assert_raises BadRenderException do
      markdown.render('hi')
    end
  end
end
