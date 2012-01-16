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
  
  EXTENSION_MAP = {
    # old name => new name
    :autolink => :autolink,
    :fenced_code => :fenced_code_blocks,
    :filter_html => :filter_html,
    :hard_wrap => :hard_wrap,
    :lax_htmlblock => :lax_html_blocks,
    :no_image => :no_images,
    :no_intraemphasis => :no_intra_emphasis,
    :no_links => :no_links,
    :filter_styles => :no_styles,
    :safelink => :safe_links_only,
    :space_header => :space_after_headers,
    :strikethrough => :strikethrough,
    :tables => :tables,
    :with_toc_data => :generate_toc,
    :xhtml => :xhtml,
    # old names with no new mapping
    :gh_blockcode => nil,
    :no_tables => nil,
    :smart => nil,
    :strict => nil
  }
  
  def rename_extensions(exts)
    exts.map do |old_name|
      if new_name = EXTENSION_MAP[old_name]
        new_name
      else
        old_name
      end
    end.compact
  end
end

