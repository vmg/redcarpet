# coding: UTF-8
require 'test_helper'

# Boundary coverage for smartypants_cb__ltag in ext/redcarpet/html_smartypants.c.
# Inputs whose inline HTML ends exactly at a tag boundary previously caused the
# callback to read one byte past the end of the input (the `bufput(ob, text,
# i + 1)` with i == size) and to run the possessive `&#39;` strncmp without
# checking the 5 bytes were in bounds. These assert correct output at those
# boundaries and that the possessive conversion still works.
class SmartyPantsBoundaryTest < Redcarpet::TestCase
  class Pants < Redcarpet::Render::HTML
    include Redcarpet::Render::SmartyPants
  end

  def setup
    @pants = Pants.new
  end

  def test_unterminated_tag_does_not_append_extra_byte
    assert_equal "hello <", @pants.postprocess("hello <")
  end

  def test_closing_a_tag_at_end_of_input
    assert_equal "see </a>", @pants.postprocess("see </a>")
    assert_equal "</a>", @pants.postprocess("</a>")
  end

  def test_well_formed_tag_is_unchanged
    assert_equal "hello <x>", @pants.postprocess("hello <x>")
  end

  def test_possessive_quote_after_closing_a_still_converts
    assert_equal %(<a href="x">Bob</a>&rsquo;s book),
                 @pants.postprocess(%(<a href="x">Bob</a>&#39;s book))
    assert_equal "</a>&rsquo;", @pants.postprocess("</a>&#39;")
  end
end
