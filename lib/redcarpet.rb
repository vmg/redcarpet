require 'redcarpet.so'

module Redcarpet
  VERSION = '2.0.0b2'

  class Markdown
    # Available Markdown extensions
    attr_accessor :no_intra_emphasis
    attr_accessor :tables
    attr_accessor :fenced_code_blocks
    attr_accessor :autolink
    attr_accessor :strikethrough
    attr_accessor :lax_html_blocks
    attr_accessor :space_after_headers
    attr_accessor :superscript

    attr_accessor :renderer

    def initialize(renderer, extensions={})
      if renderer.instance_of? Class
        renderer = renderer.new
      end

      @renderer = renderer
      extensions.each_pair { |k, v| send("#{k}=", v) }
    end
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
    #   Redcarpet::Render::SmartyPants.postprocess("you're")
    #
    module SmartyPants
      extend self
      def self.render(text)
        postprocess text
      end
    end
  end
end

