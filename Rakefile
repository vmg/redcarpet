require 'date'
require 'rake/clean'
require 'rake/extensiontask'
require 'digest/md5'

task :default => [:test]

# Ruby Extension
Rake::ExtensionTask.new('redcarpet')

# Packaging
require 'bundler/gem_tasks'

# Testing
load 'tasks/testing.rake'

# Sundown
load 'tasks/sundown.rake'