require 'redcarpet.so'

module Redcarpet
  VERSION = '2.0.0b5'

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
# Creates an instance of Redcarpet with the RedCloth API.
class RedcarpetCompat
  attr_accessor :text

  def initialize(text, *exts)
    exts_hash = rename_extensions(exts).inject({}) {|h, k| h[k] = true; h }
    @text = text
    @markdown = Redcarpet::Markdown.new(Redcarpet::Render::HTML, exts_hash)
  end

  def to_html(*_dummy)
    @markdown.render(@text)
  end
  
  private
  
  def rename_extensions(exts)
    exts.map do |ext|
      case ext
      when :gh_blockcode then nil
      when :fenced_code then :fenced_code_blocks
      when :no_intraemphasis then :no_intra_emphasis
      else ext
      end
    end.compact
  end
end

