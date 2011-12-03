require 'redcarpet'

# Compatibility class;
# Creates a instance of Redcarpet with the RedCloth
# API. This instance has no extensions enabled whatsoever,
# and no accessors to change this. 100% pure, standard
# Markdown.
class RedcarpetCompat
  attr_accessor :text

  def initialize(text, *_dummy)
    @text = text
    @markdown = Redcarpet::Markdown.new(Redcarpet::Render::HTML)
  end

  def to_html(*_dummy)
    @markdown.render(@text)
  end
end

Markdown = RedcarpetCompat unless defined? Markdown
