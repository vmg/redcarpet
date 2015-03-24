# encoding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'greenmat'

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
  # = MANIFEST =
  s.files = %w[
    COPYING
    Gemfile
    README.md
    Rakefile
    bin/greenmat
    ext/greenmat/autolink.c
    ext/greenmat/autolink.h
    ext/greenmat/buffer.c
    ext/greenmat/buffer.h
    ext/greenmat/extconf.rb
    ext/greenmat/houdini.h
    ext/greenmat/houdini_href_e.c
    ext/greenmat/houdini_html_e.c
    ext/greenmat/html.c
    ext/greenmat/html.h
    ext/greenmat/html_blocks.h
    ext/greenmat/html_smartypants.c
    ext/greenmat/markdown.c
    ext/greenmat/markdown.h
    ext/greenmat/gm_markdown.c
    ext/greenmat/gm_render.c
    ext/greenmat/greenmat.h
    ext/greenmat/stack.c
    ext/greenmat/stack.h
    lib/greenmat.rb
    lib/greenmat/compat.rb
    lib/greenmat/render_man.rb
    lib/greenmat/render_strip.rb
    greenmat.gemspec
    test/benchmark.rb
    test/custom_render_test.rb
    test/html5_test.rb
    test/html_render_test.rb
    test/html_toc_render_test.rb
    test/markdown_test.rb
    test/pathological_inputs_test.rb
    test/greenmat_compat_test.rb
    test/safe_render_test.rb
    test/smarty_html_test.rb
    test/smarty_pants_test.rb
    test/stripdown_render_test.rb
    test/test_helper.rb
  ]
  # = MANIFEST =
  s.test_files = s.files.grep(%r{^test/})
  s.extra_rdoc_files = ["COPYING"]
  s.extensions = ["ext/greenmat/extconf.rb"]
  s.executables = ["greenmat"]
  s.require_paths = ["lib"]

  s.add_development_dependency "rake-compiler", "~> 0.8.3"
  s.add_development_dependency "rspec", "~> 3.2"
  s.add_development_dependency "rubygems-xcodeproj_generator", '~> 0.1'
  s.add_development_dependency "test-unit", "~> 3.0.9"
end
