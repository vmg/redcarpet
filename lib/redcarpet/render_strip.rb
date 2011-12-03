
module Redcarpet
  module Render
    # Markdown-stripping renderer. Turns Markdown into plaintext
    # Thanks to @toupeira (Markus Koller)
    class StripDown < Base
      # Methods where the first argument is the text content
      [
        # block-level calls
        :block_code, :block_quote,
        :block_html, :header, :list,
        :list_item, :paragraph,

        # span-level calls
        :autolink, :codespan, :double_emphasis,
        :emphasis, :raw_html, :triple_emphasis,
        :strikethrough, :superscript,

        # low level rendering
        :entity, :normal_text
      ].each do |method|
        define_method method do |*args|
          args.first
        end
      end

      # Other methods where the text content is in another argument
      def link(link, title, content)
        content
      end
    end
  end
end
