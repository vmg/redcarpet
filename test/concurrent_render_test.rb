# coding: UTF-8
require 'test_helper'

class ConcurrentRenderTest < Redcarpet::TestCase
  class SlowRender < Redcarpet::Render::Base
    def paragraph(text)
      sleep 0.01
      text
    end
  end

  def setup
    @markdown = Redcarpet::Markdown.new(SlowRender)
  end

  def test_concurrent_render
    threads = (1..5).map do
      Thread.new do
        5.times do
          assert_equal 'hi', @markdown.render('hi')
        end
      end
    end
    threads.each(&:join)
  end
end
