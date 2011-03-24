Gem::Specification.new do |s|
  s.name = 'redcarpet'
  s.version = '0.1.0'
  s.summary = "Ruby bindings for libupskirt"
  s.date = '2011-03-25'
  s.email = 'vicent@github.com'
  s.homepage = 'http://github.com/tanoki/redcarpet'
  s.has_rdoc = true
  s.authors = ["Natacha Porté", "Vicent Martí"]
  # = MANIFEST =
  s.files = %w[
    BUILDING
    COPYING
    README.markdown
    Rakefile
    bin/rdiscount
    discount
    ext/Csio.c
    ext/amalloc.h
    ext/basename.c
    ext/config.h
    ext/css.c
    ext/cstring.h
    ext/docheader.c
    ext/dumptree.c
    ext/emmatch.c
    ext/extconf.rb
    ext/generate.c
    ext/html5.c
    ext/markdown.c
    ext/markdown.h
    ext/mkdio.c
    ext/mkdio.h
    ext/rdiscount.c
    ext/resource.c
    ext/tags.c
    ext/tags.h
    ext/toc.c
    ext/xml.c
    lib/markdown.rb
    lib/rdiscount.rb
    man/markdown.7
    man/rdiscount.1
    man/rdiscount.1.ronn
    rdiscount.gemspec
    test/benchmark.rb
    test/benchmark.txt
    test/markdown_test.rb
    test/rdiscount_test.rb
  ]
  # = MANIFEST =
  s.test_files = ["test/markdown_test.rb", "test/redcarpet_test.rb"]
  s.extra_rdoc_files = ["COPYING"]
  s.extensions = ["ext/extconf.rb"]
  s.executables = ["redcarpet"]
  s.require_paths = ["lib"]
end
