# coding: UTF-8
require 'test_helper'

class ExceptionSafetyTest < Redcarpet::TestCase
  # Renderer that raises an exception when rendering a link
  class ExceptionRenderer < Redcarpet::Render::HTML
    attr_accessor :should_raise

    def link(link, title, content)
      raise "Simulated exception in link callback" if @should_raise
      super
    end
  end

  def test_exception_in_callback_is_raised_to_caller
    renderer = ExceptionRenderer.new
    processor = Redcarpet::Markdown.new(renderer)

    renderer.should_raise = true
    error = assert_raise(RuntimeError) do
      processor.render("Click [here](http://example.com)")
    end
    assert_match "Simulated exception", error.message
  end

  def test_processor_recovers_after_exception
    renderer = ExceptionRenderer.new
    processor = Redcarpet::Markdown.new(renderer)

    # First render raises exception
    renderer.should_raise = true
    assert_raise(RuntimeError) do
      processor.render("Click [here](http://example.com)")
    end

    # Next render works (cleanup reset the state)
    renderer.should_raise = false
    result = processor.render("Hello **world**!")
    assert_match "<strong>world</strong>", result
  end

  def test_multiple_exceptions_and_recoveries
    renderer = ExceptionRenderer.new
    processor = Redcarpet::Markdown.new(renderer)

    3.times do |i|
      # Raise exception
      renderer.should_raise = true
      assert_raise(RuntimeError) do
        processor.render("[link](url)")
      end

      # Immediate recovery works
      renderer.should_raise = false
      result = processor.render("Normal text #{i}")
      assert_match "Normal text #{i}", result
    end
  end

  def test_normal_render_does_not_raise
    # Verify that normal renders without exceptions work fine
    renderer = Redcarpet::Render::HTML.new
    processor = Redcarpet::Markdown.new(renderer)

    100.times do |i|
      result = processor.render("Test **#{i}** with [link](url)")
      assert_match "<strong>#{i}</strong>", result
      assert_match "href", result
    end
  end

  # Renderer that sleeps during callback to increase threading race likelihood
  class SlowRenderer < Redcarpet::Render::HTML
    def link(link, title, content)
      sleep(0.001)  # Brief sleep while inside render, holding work_bufs
      %(<a href="#{link}">#{content}</a>)
    end
  end

  def test_concurrent_use_detects_threading_issues
    # Concurrent use of a single Markdown instance is unsafe.
    # This test verifies we either:
    # - Complete without issue (race didn't manifest), or
    # - Detect the problem and raise the specific imbalance exception
    # We should never crash or raise an unexpected exception.
    #
    # SlowRenderer sleeps during callbacks to increase the chance that
    # multiple threads are inside sd_markdown_render simultaneously.

    renderer = SlowRenderer.new
    processor = Redcarpet::Markdown.new(renderer)

    imbalance_detected = false
    unexpected_error = nil
    mutex = Mutex.new
    barrier = Queue.new

    threads = 8.times.map do |t|
      Thread.new do
        barrier.pop  # Wait for start signal
        20.times do |i|
          begin
            processor.render("[link](url) and **bold** and [another](link)")
          rescue RuntimeError => e
            if e.message.include?("work buffer imbalance")
              mutex.synchronize { imbalance_detected = true }
            else
              mutex.synchronize { unexpected_error ||= e }
            end
          rescue => e
            mutex.synchronize { unexpected_error ||= e }
          end
        end
      end
    end

    # Start all threads simultaneously
    8.times { barrier << true }
    threads.each(&:join)

    # Fail if we got an unexpected exception
    assert_nil unexpected_error, "Unexpected error: #{unexpected_error&.message}"

    # Uncomment the following if you want to see this detail:
    # puts "Note: thread imbalance correctly detected" if imbalance_detected
    # Note: imbalance_detected may or may not be true depending on timing.
    # Either outcome is acceptable - we just verify no crashes or unexpected errors.
  end
end
