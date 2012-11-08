Fork of Markdown 2 that supports everything it does, plus footnotes.

Footnote-parsing Sundown code grabbed hastily from @bdolman's [pull request](https://github.com/vmg/sundown/pull/141),
which has not yet been accepted into Sundown at the time of writing.

This fork of Redcarpet additionally adds the following callbacks:

    # block-level
    footnotes(content)
    footnote_def(content, footnote_index)

    # span-level
    footnote_ref(content, footnote_index)
