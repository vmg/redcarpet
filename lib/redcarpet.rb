require 'redcarpet.so'

module Redcarpet
  VERSION = '2.0.0'

  class Markdown
    attr_reader :renderer
  end

  module Render

    # XHTML Renderer
    class XHTML < HTML
      def initialize(extensions={})
        super(extensions.merge(:xhtml => true))
      end
    end

    # HTML + SmartyPants renderer
    class SmartyHTML < HTML
      include SmartyPants
    end

    # SmartyPants Mixin module
    #
    # Implements SmartyPants.postprocess, which
    # performs smartypants replacements on the HTML file,
    # once it has been fully rendered.
    #
    # To add SmartyPants postprocessing to your custom
    # renderers, just mixin the module `include SmartyPants`
    #
    # You can also use this as a standalone SmartyPants
    # implementation.
    #
    # Example:
    #
    #   # Mixin
    #   class CoolRenderer < HTML
    #     include SmartyPants
    #     # more code here
    #   end
    #
    #   # Standalone
    #   Redcarpet::Render::SmartyPants.render("you're")
    #
    module SmartyPants
      extend self
      def self.render(text)
        postprocess text
      end
    end
  end
end

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

