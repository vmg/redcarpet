# encoding: utf-8
rootdir = File.dirname(File.dirname(__FILE__))
$LOAD_PATH.unshift "#{rootdir}/lib"

require 'test/unit'
require 'redcarpet'

class RedcarpetTest < Test::Unit::TestCase
  def test_that_discount_does_not_blow_up_with_weird_formatting_case
    text = (<<-TEXT).gsub(/^ {4}/, '').rstrip
    1. some text

    1.
    TEXT
    Redcarpet.new(text).to_html
  end

  def test_that_smart_converts_double_quotes_to_curly_quotes
    rd = Redcarpet.new(%("Quoted text"), :smart)
    assert_equal %(<p>&ldquo;Quoted text&rdquo;</p>\n), rd.to_html
  end

  def test_that_smart_converts_double_quotes_to_curly_quotes_before_a_heading
    rd = Redcarpet.new(%("Quoted text"\n\n# Heading), :smart)
    assert_equal %(<p>&ldquo;Quoted text&rdquo;</p>\n\n<h1>Heading</h1>\n), rd.to_html
  end

  def test_that_smart_converts_double_quotes_to_curly_quotes_after_a_heading
    rd = Redcarpet.new(%(# Heading\n\n"Quoted text"), :smart)
    assert_equal %(<h1>Heading</h1>\n\n<p>&ldquo;Quoted text&rdquo;</p>\n), rd.to_html
  end

  def test_that_smart_gives_ve_suffix_a_rsquo
    rd = Redcarpet.new("I've been meaning to tell you ..", :smart)
    assert_equal "<p>I&rsquo;ve been meaning to tell you ..</p>\n", rd.to_html
  end

  def test_that_smart_gives_m_suffix_a_rsquo
    rd = Redcarpet.new("I'm not kidding", :smart)
    assert_equal "<p>I&rsquo;m not kidding</p>\n", rd.to_html
  end

  def test_that_smart_gives_d_suffix_a_rsquo
    rd = Redcarpet.new("what'd you say?", :smart)
    assert_equal "<p>what&rsquo;d you say?</p>\n", rd.to_html
  end

  if "".respond_to?(:encoding)
    def test_should_return_string_in_same_encoding_as_input
      input = "Yogācāra"
      output = Redcarpet.new(input).to_html
      assert_equal input.encoding.name, output.encoding.name
    end
  end

  def test_that_no_image_flag_works
    rd = Redcarpet.new(%(![dust mite](http://dust.mite/image.png) <img src="image.png" />), :no_image)
    assert rd.to_html !~ /<img/
  end

  def test_that_no_links_flag_works
    rd = Redcarpet.new(%([This link](http://example.net/) <a href="links.html">links</a>), :no_links)
    assert rd.to_html !~ /<a /
  end

  def test_that_strict_flag_works
    rd = Redcarpet.new("foo_bar_baz", :strict)
    assert_equal "<p>foo<em>bar</em>baz</p>\n", rd.to_html
  end

  def test_that_autolink_flag_works
    rd = Redcarpet.new("Check this out at http://github.com/rtomayko/rdiscount and don't forget to ", :autolink)
    assert_equal "<p><a href=\"http://github.com/rtomayko/rdiscount\">http://github.com/rtomayko/rdiscount</a></p>\n", rd.to_html
  end

  def test_that_safelink_flag_works
    rd = Redcarpet.new("[IRC](irc://chat.freenode.org/#freenode)", :safelink)
    assert_equal "<p>[IRC](irc://chat.freenode.org/#freenode)</p>\n", rd.to_html
  end

  def test_that_tags_can_have_dashes_and_underscores
    rd = Redcarpet.new("foo <asdf-qwerty>bar</asdf-qwerty> and <a_b>baz</a_b>")
    assert_equal "<p>foo <asdf-qwerty>bar</asdf-qwerty> and <a_b>baz</a_b></p>\n", rd.to_html
  end
  
  def xtest_pathological_1
    star = '*'  * 250000
    Redcarpet.new("#{star}#{star} hi #{star}#{star}").to_html
  end

  def xtest_pathological_2
    crt = '^' * 255
    str = "#{crt}(\\)"
    Redcarpet.new("#{str*300}").to_html
  end

  def xtest_pathological_3
    c = "`t`t`t`t`t`t" * 20000000
    Redcarpet.new(c).to_html
  end

  def xtest_pathological_4
    Redcarpet.new(" [^a]: #{ "A" * 10000 }\n#{ "[^a][]" * 1000000 }\n").to_html.size
  end

end
