# Standalone script to test GC compaction safety.
# Invoked as a subprocess by gc_compaction_test.rb.
#
# This script must run in a separate process because:
# 1. Test::Unit framework's heap state prevents reliable bug triggering
# 2. Subprocess protects the test runner from segfaults

require "redcarpet"

renderer = Redcarpet::Render::HTML.new(
  link_attributes: { rel: "nofollow ugc noopener noreferrer" }
)
processor = Redcarpet::Markdown.new(renderer)

# Verify it works before compaction
result1 = processor.render("[link](http://test.com)")
unless result1.include?('rel="nofollow')
  STDERR.puts "FAIL: link_attributes not working before compaction"
  exit 1
end

# Create garbage then compact - this moves objects in heap
100_000.times { "x" * 100 }
GC.verify_compaction_references(expand_heap: true, toward: :empty)

# Verify it still works after compaction
# Without the compact callback fix, this crashes with segfault
result2 = processor.render("[link2](http://example.com)")
unless result2.include?('rel="nofollow')
  STDERR.puts "FAIL: link_attributes not working after compaction"
  exit 1
end

puts "PASS"
exit 0
