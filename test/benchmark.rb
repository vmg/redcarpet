# Thanks Kramdown for the inspiration!
require "benchmark"
require "stringio"

require 'redcarpet'
require 'kramdown'
require 'bluecloth'
require 'rdiscount'

TEST = 10_000
m = File.read(File.join(File.dirname(__FILE__), "fixture.text"))

# Let's bench!
Benchmark.bm do |bench|
  bench.report("Redcarpet") do
    TEST.times { Redcarpet::Markdown.new(Redcarpet::Render::HTML).render(m) }
  end

  bench.report("BlueCoth") do
    TEST.times { BlueCloth.new(m).to_html }
  end

  bench.report("RDiscount") do
    TEST.times { RDiscount.new(m).to_html }
  end

  bench.report("Kramdown") do
    TEST.times { Kramdown::Document.new(m).to_html }
  end
end
