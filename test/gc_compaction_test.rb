# frozen_string_literal: true

require 'test_helper'

class GCCompactionTest < Redcarpet::TestCase
  def setup
    skip "GC compaction not available" unless GC.respond_to?(:verify_compaction_references)
  end

  def test_gc_compaction_with_link_attributes
    runner = File.expand_path('gc_compaction_runner.rb', __dir__)
    output = IO.popen([RbConfig.ruby, "-I#{File.expand_path('../lib', __dir__)}", runner], err: [:child, :out], &:read)
    assert $?.success?, "GC compaction test failed:\n#{output}"
    assert_match(/PASS/, output)
  end
end
