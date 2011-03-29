require 'date'
require 'rake/clean'
require 'digest/md5'

task :default => :test

# ==========================================================
# Ruby Extension
# ==========================================================

DLEXT = Config::MAKEFILE_CONFIG['DLEXT']
RUBYDIGEST = Digest::MD5.hexdigest(`#{RUBY} --version`)

file "ext/ruby-#{RUBYDIGEST}" do |f|
  rm_f FileList["ext/ruby-*"]
  touch f.name
end
CLEAN.include "ext/ruby-*"

file 'ext/Makefile' => FileList['ext/*.{c,h,rb}', "ext/ruby-#{RUBYDIGEST}"] do
  chdir('ext') { ruby 'extconf.rb' }
end
CLEAN.include 'ext/Makefile', 'ext/mkmf.log'

file "ext/redcarpet.#{DLEXT}" => FileList["ext/Makefile"] do |f|
  sh 'cd ext && make clean && make && rm -rf conftest.dSYM'
end
CLEAN.include 'ext/*.{o,bundle,so,dll}'

file "lib/redcarpet.#{DLEXT}" => "ext/redcarpet.#{DLEXT}" do |f|
  cp f.prerequisites, "lib/", :preserve => true
end

desc 'Build the redcarpet extension'
task :build => "lib/redcarpet.#{DLEXT}"

# ==========================================================
# Testing
# ==========================================================

require 'rake/testtask'
Rake::TestTask.new('test:unit') do |t|
  t.test_files = FileList['test/*_test.rb']
  t.ruby_opts += ['-rubygems'] if defined? Gem
end
task 'test:unit' => [:build]

desc 'Run conformance tests (MARKDOWN_TEST_VER=1.0)'
task 'test:conformance' => [:build] do |t|
  script = "#{pwd}/bin/redcarpet"
  test_version = ENV['MARKDOWN_TEST_VER'] || '1.0.3'
  lib_dir = "#{pwd}/lib"
  chdir("test/MarkdownTest_#{test_version}") do
    sh "RUBYLIB=#{lib_dir} ./MarkdownTest.pl --script='#{script}' --tidy"
  end
end

desc 'Run version 1.0 conformance suite'
task 'test:conformance:1.0' => [:build] do |t|
  ENV['MARKDOWN_TEST_VER'] = '1.0'
  Rake::Task['test:conformance'].invoke
end

desc 'Run 1.0.3 conformance suite'
task 'test:conformance:1.0.3' => [:build] do |t|
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

desc 'Update the gemspec'
task :update_gem => file('redcarpet.gemspec')

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

file 'redcarpet.gemspec' => FileList['Rakefile','lib/redcarpet.rb'] do |f|
  # read spec file and split out manifest section
  spec = File.read(f.name)
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
  File.open(f.name, 'w') { |io| io.write(spec) }
  puts "updated #{f.name}"
end
