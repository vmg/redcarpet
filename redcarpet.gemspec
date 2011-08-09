Gem::Specification.new do |s|
  s.name = 'redcarpet'
  s.version = '2.0.0b3'
  s.summary = "Markdown that smells nice"
  s.description = 'A fast, safe and extensible Markdown to (X)HTML parser'
  s.date = '2011-08-09'
  s.email = 'vicent@github.com'
  s.homepage = 'http://github.com/tanoku/redcarpet'
  s.authors = ["Natacha Porté", "Vicent Martí"]
  # = MANIFEST =
  s.files = %w[
    COPYING
    README.markdown
    Rakefile
    bin/redcarpet
    ext/redcarpet/array.c
    ext/redcarpet/array.h
    ext/redcarpet/autolink.c
    ext/redcarpet/autolink.h
    ext/redcarpet/buffer.c
    ext/redcarpet/buffer.h
    ext/redcarpet/extconf.rb
    ext/redcarpet/html.c
    ext/redcarpet/html.h
    ext/redcarpet/html_smartypants.c
    ext/redcarpet/markdown.c
    ext/redcarpet/markdown.h
    ext/redcarpet/rc_markdown.c
    ext/redcarpet/rc_render.c
    ext/redcarpet/redcarpet.h
    lib/redcarpet.rb
    lib/redcarpet/render_man.rb
    redcarpet.gemspec
    sundown
    test/redcarpet_test.rb
  ]
  # = MANIFEST =
  s.test_files = ["test/redcarpet_test.rb"]
  s.extra_rdoc_files = ["COPYING"]
  s.extensions = ["ext/redcarpet/extconf.rb"]
  s.executables = ["redcarpet"]
  s.require_paths = ["lib"]
  s.add_development_dependency "rake-compiler"
end
