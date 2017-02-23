# encoding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'greenmat/version'

Gem::Specification.new do |s|
  s.name = 'greenmat'
  s.version = Greenmat::VERSION
  s.summary = "A Markdown parser for Qiita, based on Redcarpet."
  s.description = s.summary
  s.email = 'nkymyj@gmail.com'
  s.homepage = 'https://github.com/increments/greenmat'
  s.authors = ["Natacha Porté", "Vicent Martí"]
  s.license = 'MIT'
  s.required_ruby_version = '>= 2.0.0'

  s.files = `git ls-files -z`.split("\x0")
  s.test_files = s.files.grep(%r{^(test|spec|features)/})
  s.extra_rdoc_files = ["COPYING"]
  s.extensions = ["ext/greenmat/extconf.rb"]
  s.executables = ["greenmat"]
  s.require_paths = ["lib"]

  s.add_development_dependency "nokogiri", "~> 1.6.0"
  s.add_development_dependency "rake", "~> 10.0"
  s.add_development_dependency "rake-compiler", "~> 0.8.3"
  s.add_development_dependency "rspec", "~> 3.2"
  s.add_development_dependency "test-unit", "~> 2.5.4"
end
