Gem::Specification.new do |s|
  s.name = 'redcarpet'
  s.version = '1.5.1'
  s.summary = "Ruby bindings for libupskirt"
  s.date = '2011-04-11'
  s.email = 'vicent@github.com'
  s.homepage = 'http://github.com/tanoku/redcarpet'
  s.has_rdoc = true
  s.authors = ["Natacha Porté", "Vicent Martí"]
  # = MANIFEST =
  s.files = %w[
    COPYING
    README.markdown
    Rakefile
    bin/redcarpet
    ext/array.c
    ext/array.h
    ext/buffer.c
    ext/buffer.h
    ext/extconf.rb
    ext/markdown.c
    ext/markdown.h
    ext/redcarpet.c
    ext/xhtml.c
    ext/xhtml.h
    lib/markdown.rb
    lib/redcarpet.rb
    redcarpet.gemspec
    test/benchmark.rb
    test/benchmark.txt
    test/markdown_test.rb
    test/redcarpet_test.rb
  ]
  # = MANIFEST =
  s.test_files = ["test/markdown_test.rb", "test/redcarpet_test.rb"]
  s.extra_rdoc_files = ["COPYING"]
  s.extensions = ["ext/extconf.rb"]
  s.executables = ["redcarpet"]
  s.require_paths = ["lib"]
end
