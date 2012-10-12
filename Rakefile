require 'date'
require 'rake/clean'
require 'rake/extensiontask'
require 'digest/md5'

task :default => [:test]

# ==========================================================
# Ruby Extension
# ==========================================================

Rake::ExtensionTask.new('redcarpet')

# ==========================================================
# Testing
# ==========================================================

require 'rake/testtask'
Rake::TestTask.new('test:unit') do |t|
  t.test_files = FileList['test/*_test.rb']
  t.ruby_opts += ['-rubygems'] if defined? Gem
end
task 'test:unit' => [:compile]

desc 'Run conformance tests (MARKDOWN_TEST_VER=1.0)'
task 'test:conformance' => [:compile] do |t|
  script = "#{pwd}/bin/redcarpet"
  test_version = ENV['MARKDOWN_TEST_VER'] || '1.0.3'
  lib_dir = "#{pwd}/lib"
  chdir("test/MarkdownTest_#{test_version}") do
    sh "RUBYLIB=#{lib_dir} ./MarkdownTest.pl --script='#{script}' --tidy"
  end
end

desc 'Run version 1.0 conformance suite'
task 'test:conformance:1.0' => [:compile] do |t|
  ENV['MARKDOWN_TEST_VER'] = '1.0'
  Rake::Task['test:conformance'].invoke
end

desc 'Run 1.0.3 conformance suite'
task 'test:conformance:1.0.3' => [:compile] do |t|
  ENV['MARKDOWN_TEST_VER'] = '1.0.3'
  Rake::Task['test:conformance'].invoke
end

desc 'Run unit and conformance tests'
task :test => %w[test:unit test:conformance]

desc 'Run benchmarks'
task :benchmark => :build do |t|
  $:.unshift 'lib'
  load 'test/benchmark.rb'
end

# PACKAGING =================================================================

require 'rubygems'
$spec = eval(File.read('redcarpet.gemspec'))

def package(ext='')
  "pkg/redcarpet-#{$spec.version}" + ext
end

desc 'Build packages'
task :package => %w[.gem .tar.gz].map {|e| package(e)}

desc 'Build and install as local gem'
task :install => package('.gem') do
  sh "gem install #{package('.gem')}"
end

directory 'pkg/'

file package('.gem') => %w[pkg/ redcarpet.gemspec] + $spec.files do |f|
  sh "gem build redcarpet.gemspec"
  mv File.basename(f.name), f.name
end

file package('.tar.gz') => %w[pkg/] + $spec.files do |f|
  sh "git archive --format=tar HEAD | gzip > #{f.name}"
end

# GEMSPEC HELPERS ==========================================================

def source_version
  line = File.read('lib/redcarpet.rb')[/^\s*VERSION = .*/]
  line.match(/.*VERSION = '(.*)'/)[1]
end

task :update_gem do
  # read spec file and split out manifest section
  GEMFILE = 'redcarpet.gemspec'
  spec = File.read(GEMFILE)
  head, manifest, tail = spec.split("  # = MANIFEST =\n")
  head.sub!(/\.version = '.*'/, ".version = '#{source_version}'")
  head.sub!(/\.date = '.*'/, ".date = '#{Date.today.to_s}'")
  # determine file list from git ls-files
  files = `git ls-files`.
    split("\n").
    sort.
    reject{ |file| file =~ /^\./ || file =~ /^test\/MarkdownTest/ }.
    map{ |file| "    #{file}" }.
    join("\n")
  # piece file back together and write...
  manifest = "  s.files = %w[\n#{files}\n  ]\n"
  spec = [head,manifest,tail].join("  # = MANIFEST =\n")
  File.open(GEMFILE, 'w') { |io| io.write(spec) }
  puts "updated #{GEMFILE}"
end

desc 'Gather required Sundown sources into extension directory'
task :gather => 'sundown:checkout' do |t|
  files =
    FileList[
      'sundown/src/{markdown,buffer,stack,autolink,html_blocks}.h',
      'sundown/src/{markdown,buffer,stack,autolink}.c',
      'sundown/html/{html,html_smartypants,houdini_html_e,houdini_href_e}.c',
      'sundown/html/{html,houdini}.h',
    ]
  cp files, 'ext/redcarpet/',
    :preserve => true,
    :verbose => true
end

task 'sundown:checkout' do |t|
  unless File.exists?('sundown/src/markdown.h')
    sh 'git submodule init'
    sh 'git submodule update'
  end
end
