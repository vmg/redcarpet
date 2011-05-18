Gem::Specification.new do |s|
  s.name = 'redcarpet'
  s.version = '1.14.1'
  s.summary = "Ruby bindings for libupskirt"
  s.description = 'A fast and safe Markdown to (X)HTML parser'
  s.date = '2011-05-18'
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
    ext/redcarpet/buffer.c
    ext/redcarpet/buffer.h
    ext/redcarpet/extconf.rb
    ext/redcarpet/html.c
    ext/redcarpet/html.h
    ext/redcarpet/html_smartypants.c
    ext/redcarpet/markdown.c
    ext/redcarpet/markdown.h
    ext/redcarpet/redcarpet.c
    lib/markdown.rb
    lib/redcarpet.rb
    redcarpet.gemspec
    test/benchmark.rb
    test/benchmark.txt
    test/markdown_test.rb
    test/redcarpet_test.rb
    upskirt
  ]
  # = MANIFEST =
  s.test_files = ["test/markdown_test.rb", "test/redcarpet_test.rb"]
  s.extra_rdoc_files = ["COPYING"]
  s.extensions = ["ext/redcarpet/extconf.rb"]
  s.executables = ["redcarpet"]
  s.require_paths = ["lib"]
end
