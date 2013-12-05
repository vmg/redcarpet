begin
  require 'pygments'
  PYGMENTS_AVAILABLE = true
rescue LoadError
  PYGMENTS_AVAILABLE = false
end

module Redcarpet
  module Render
    class HTMLwithPygments < HTML
      def block_code(code, language)
        # TODO Move this to init or setup block.
        if not PYGMENTS_AVAILABLE
          raise "Pygments.rb must be installed."
        end
        Pygments.highlight(code, :lexer => language)
      end
    end
  end
end
