# encoding: utf-8
Gem::Specification.new do |s|
  s.name = 'redcarpet'
  s.version = '2.3.0'
  s.summary = "Markdown that smells nice"
  s.description = 'A fast, safe and extensible Markdown to (X)HTML parser'
  s.date = '2013-05-22'
  s.email = 'vicent@github.com'
  s.homepage = 'http://github.com/vmg/redcarpet'
  s.authors = ["Natacha Porté", "Vicent Martí"]
  # = MANIFEST =
  s.files = %w[
    COPYING
    Gemfile
    README.markdown
    Rakefile
    bin/redcarpet
    ext/redcarpet/autolink.c
    ext/redcarpet/autolink.h
    ext/redcarpet/buffer.c
    ext/redcarpet/buffer.h
    ext/redcarpet/extconf.rb
    ext/redcarpet/houdini.h
    ext/redcarpet/houdini_href_e.c
    ext/redcarpet/houdini_html_e.c
    ext/redcarpet/html.c
    ext/redcarpet/html.h
    ext/redcarpet/html_blocks.h
    ext/redcarpet/html_smartypants.c
    ext/redcarpet/markdown.c
    ext/redcarpet/markdown.h
    ext/redcarpet/rc_markdown.c
    ext/redcarpet/rc_render.c
    ext/redcarpet/redcarpet.h
    ext/redcarpet/stack.c
    ext/redcarpet/stack.h
    lib/redcarpet.rb
    lib/redcarpet/compat.rb
    lib/redcarpet/render_man.rb
    lib/redcarpet/render_strip.rb
    redcarpet.gemspec
    sundown
    test/test_helper.rb
    test/custom_render_test.rb
    test/html_render_test.rb
    test/markdown_test.rb
    test/pathological_inputs_test.rb
    test/redcarpet_compat_test.rb
    test/smarty_html_test.rb
    test/smarty_pants_test.rb
    test/stripdown_render_test.rb
  ]
  # = MANIFEST =
  s.test_files = s.files.grep(%r{^test/})
  s.extra_rdoc_files = ["COPYING"]
  s.extensions = ["ext/redcarpet/extconf.rb"]
  s.executables = ["redcarpet"]
  s.require_paths = ["lib"]

  s.add_development_dependency "nokogiri"
  s.add_development_dependency "rake-compiler"
  s.add_development_dependency "test-unit"
  s.add_development_dependency "bluecloth"
  s.add_development_dependency "kramdown"
end
