require 'rubygems'

iterations = 100
test_file = "#{File.dirname(__FILE__)}/benchmark.txt"
implementations = %w[BlueCloth RDiscount Maruku PEGMarkdown Redcarpet]

# Attempt to require each implementation and remove any that are not
# installed.
implementations.reject! do |class_name|
  begin
    module_path =
      if class_name == 'PEGMarkdown'
        'peg_markdown'
      else
        class_name.downcase
      end
    require module_path
    false
  rescue LoadError => boom
    module_path.tr! '_', '-'
    puts "#{class_name} excluded. Try: gem install #{module_path}"
    true
  end
end

# Grab actual class objects.
implementations.map! { |class_name| Object.const_get(class_name) }

# The actual benchmark.
def benchmark(implementation, text, iterations)
  start = Time.now
  iterations.times do |i|
    implementation.new(text).to_html
  end
  Time.now - start
end

# Read test file
test_data = File.read(test_file)

# Prime the pump
puts "Spinning up ..."
implementations.each { |impl| benchmark(impl, test_data, 1) }

# Run benchmarks; gather results.
puts "Running benchmarks ..."
results =
  implementations.inject([]) do |r,impl|
    GC.start
    r << [ impl, benchmark(impl, test_data, iterations) ]
  end

puts "Results for #{iterations} iterations:"
results.each do |impl,time|
  printf "  %10s %09.06fs total time, %09.06fs average\n", "#{impl}:", time, time / iterations
end
