# coding: UTF-8
# Thanks Kramdown for the inspiration!
require 'benchmark/ips'

require 'greenmat'
require 'bluecloth'
require 'kramdown'

markdown = File.read(File.join(File.dirname(__FILE__), "fixtures/benchmark.md"))

# Let's bench!
Benchmark.ips do |bench|
  bench.report("Greenmat") do
    Greenmat::Markdown.new(Greenmat::Render::HTML).render(markdown)
  end

  bench.report("BlueCloth") do
    BlueCloth.new(markdown).to_html
  end

  bench.report("Kramdown") do
    Kramdown::Document.new(markdown).to_html
  end
end
