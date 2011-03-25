# Upskirt is an implementation of John Gruber's Markdown markup
# language. Upskirt is safe, fast and production ready.
#
# Redcarpet is Upskirt with a touch of Ruby. It is mostly based on Ryan
# Tomayko's RDiscount, and inspired by Rick Astley wearing a kilt.
#
# Redcarpet is a drop-in replacement for BlueCloth, RedCloth and RDiscount.
#
# == Usage
#
# Redcarpet implements the basic protocol popularized by RedCloth and adopted
# by BlueCloth:
#   require 'redcarpet'
#   markdown = Redcarpet.new("Hello World!")
#   puts markdown.to_html
#
# == Replacing BlueCloth
#
# Inject Redcarpet into your BlueCloth-using code by replacing your bluecloth
# require statements with the following:
#   begin
#     require 'redcarpet'
#     BlueCloth = Redcarpet
#   rescue LoadError
#     require 'bluecloth'
#   end
#
class Redcarpet
  VERSION = '0.1.0'

  # Original Markdown formatted text.
  attr_reader :text

  # Set true to have smarty-like quote translation performed.
  attr_accessor :smart

  # Do not output <tt><style></tt> tags included in the source text.
  attr_accessor :filter_styles

  attr_accessor :fold_lines # Ignore, just for compatibility

  # Do not output any raw HTML included in the source text.
  attr_accessor :filter_html

  # Do not process <tt>![]</tt> and remove <tt><img></tt> tags from the output.
  attr_accessor :no_image

  # Do not process <tt>[]</tt> and remove <tt><a></tt> tags from the output.
  attr_accessor :no_links

  # Disable superscript and relaxed emphasis processing.
  attr_accessor :strict #TODO 

  # Convert URL in links, even if they aren't encased in <tt><></tt>
  attr_accessor :autolink

  # Don't make hyperlinks from <tt>[][]</tt> links that have unknown URL types.
  attr_accessor :safelink

  def initialize(text, *extensions)
    @text  = text
    extensions.each { |e| send("#{e}=", true) }
  end

  def generate_toc
    raise NotImplementedError
  end
end

Markdown = Redcarpet unless defined? Markdown

require 'redcarpet.so'
