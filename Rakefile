require 'date'
require 'rake/clean'
require 'rake/extensiontask'
require 'digest/md5'

task :default => [:test]

# Ruby Extension
Rake::ExtensionTask.new('redcarpet')

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

# Testing
load 'tasks/testing.rake'

# Sundown
load 'tasks/sundown.rake'