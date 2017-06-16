require 'greenmat'
require "active_support/core_ext/string/strip"

module Greenmat
  RSpec.describe Markdown do
    subject(:markdown) { Markdown.new(renderer, options) }
    let(:renderer) { Render::HTML.new }
    let(:options) { {} }
    let(:rendered_html) { markdown.render(text) }

    context 'with no_mention_emphasis option' do
      let(:options) { { no_mention_emphasis: true } }

      [
        ['@_username_',         false],
        ['@__username__',       false],
        ['@___username___',     false],
        ['@user__name__',       false],
        ['@some__user__name__', false],
        [' @_username_',        false],
        ['あ@_username_',       false],
        ['A@_username_',        true],
        ['@*username*',         true],
        ['_foo_',               true],
        ['_',                   false],
        ['_foo @username_',     false],
        ['__foo @username__',   false],
        ['___foo @username___', false]
      ].each do |text, emphasize|
        context "with text #{text.inspect}" do
          let(:text) { text }

          if emphasize
            it 'emphasizes the text' do
              expect(rendered_html).to include('<em>').or include('<strong>')
            end
          else
            it 'does not emphasize the text' do
              expect(rendered_html.chomp).to eq("<p>#{text.strip}</p>")
            end
          end
        end
      end
    end

    context 'without no_mention_emphasis option' do
      let(:options) { {} }

      context 'with text "@_username_"' do
        let(:text) { '@_username_' }

        it 'emphasizes the text' do
          expect(rendered_html).to include('<em>')
        end
      end

      context 'with text "_foo @username_"' do
        let(:text) { '_foo @username_' }

        it 'emphasizes the text' do
          expect(rendered_html).to include('<em>')
        end
      end
    end

    context 'with fenced_code_blocks option' do
      let(:options) { { fenced_code_blocks: true } }

      context 'with language and filename syntax' do
        let(:text) do
          <<-EOS.strip_heredoc
            ```ruby:example.rb
            puts :foo
            ```
          EOS
        end

        it 'generates <code> tag with data-metadata attribute' do
          expect(rendered_html).to eq <<-EOS.strip_heredoc
            <pre><code data-metadata="ruby:example.rb">puts :foo
            </code></pre>
          EOS
        end
      end
    end
  end
end
