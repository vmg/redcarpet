require 'rake/testtask'

Rake::TestTask.new('test:unit') do |t|
  t.test_files = FileList['test/*_test.rb']
  t.ruby_opts += ['-rubygems'] if defined? Gem
end

task 'test:unit' => :compile

desc 'Run conformance tests (MARKDOWN_TEST_VER=1.0)'
task 'test:conformance' => :compile do |t|
  script = "#{pwd}/bin/redcarpet"
  test_version = ENV['MARKDOWN_TEST_VER'] || '1.0.3'
  lib_dir = "#{pwd}/lib"

  chdir("test/MarkdownTest_#{test_version}") do
    sh "RUBYLIB=#{lib_dir} ./MarkdownTest.pl --script='#{script}' --tidy"
  end
end

desc 'Run version 1.0 conformance suite'
task 'test:conformance:1.0' => :compile do |t|
  ENV['MARKDOWN_TEST_VER'] = '1.0'
  Rake::Task['test:conformance'].invoke
end

desc 'Run 1.0.3 conformance suite'
task 'test:conformance:1.0.3' => :compile do |t|
  ENV['MARKDOWN_TEST_VER'] = '1.0.3'
  Rake::Task['test:conformance'].invoke
end

desc 'Run unit and conformance tests'
task :test => %w[test:unit test:conformance]


desc 'Run benchmarks'
task :benchmark => :compile do |t|
  $:.unshift 'lib'
  load 'test/benchmark.rb'
end
