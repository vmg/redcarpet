require 'redcarpet.so'

module Redcarpet
  VERSION = '2.2.2'

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
    exts_hash, render_hash = *parse_extensions_and_renderer_options(exts)
    @text = text
    renderer = Redcarpet::Render::HTML.new(render_hash)
    @markdown = Redcarpet::Markdown.new(renderer, exts_hash)
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
    :prettify => :prettify,
    :lax_htmlblock => :lax_spacing,
    :no_image => :no_images,
    :no_intraemphasis => :no_intra_emphasis,
    :no_links => :no_links,
    :filter_styles => :no_styles,
    :safelink => :safe_links_only,
    :space_header => :space_after_headers,
    :strikethrough => :strikethrough,
    :tables => :tables,
    :generate_toc => :with_toc_data,
    :xhtml => :xhtml,
    # old names with no new mapping
    :gh_blockcode => nil,
    :no_tables => nil,
    :smart => nil,
    :strict => nil
  }
  
  RENDERER_OPTIONS = [:filter_html, :no_images, :no_links, :no_styles, 
    :safe_links_only, :with_toc_data, :hard_wrap, :prettify, :xhtml]
  
  def rename_extensions(exts)
    exts.map do |old_name|
      if new_name = EXTENSION_MAP[old_name]
        new_name
      else
        old_name
      end
    end.compact
  end
  
  # Returns two hashes, the extensions and renderer options
  # given the extension list
  def parse_extensions_and_renderer_options(exts)
    exts = rename_extensions(exts)
    exts.partition {|ext| !RENDERER_OPTIONS.include?(ext) }.
      map {|list| list_to_truthy_hash(list) }
  end
  
  # Turns a list of symbols into a hash of <tt>symbol => true</tt>.
  def list_to_truthy_hash(list)
    list.inject({}) {|h, k| h[k] = true; h }
  end
end

