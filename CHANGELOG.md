# Changelog

* Add optional quote support through the `:quote` option. Render
  quotations marks to `q` HTML tag.

  This is a `"quote"`.

  *Anatol Broder*

* Ensure inline markup in titles is correctly stripped when generating
  headers' anchor.

  *Robin Dupret*

* Revert the unescaping behavior on comments

  This behavior doesn't follow the conformance suite.

  *Robin Dupret*

* Update the conformance test suite to provide better feedback to CI
  systems.

  *Robin Dupret*

* Add optional footnotes support

  Add PHP-Markdown style footnotes through the `:footnotes` option.

  *Ben Dolman, Adam Florin, microjo, brief*

* Enable GitHub style anchors for headers

  Passing the `with_toc_data` option to a `HTML` render object now
  generates GitHub style anchors.

  *Matt Rogers*

* Allow to set a maximum rendering level for HTML_TOC

  Allow the user to pass a `nesting_level` option when instantiating a
  new HTML_TOC render object in order to limit the nesting level in the
  generated table of content. For example:

  ~~~ruby
  Redcarpet::Markdown.new(Redcarpet::Render::HTML_TOC.new(nesting_level: 2))
  ~~~

  *Robin Dupret*

## Version 3.0.0

* Remove support for Ruby 1.8.x *Matt Rogers & Robin Dupret*

* Avoid escaping for HTML comments *Robin Dupret*

* Make emphasis wrapped inside parenthesis parsed *Robin Dupret*

* Remove the Sundown submodule *Robin Dupret*

* Fix FTP uris identified as emails *Robin Dupret*

* Add optional highlight support *Sam Soffes*

  This is `==highlighted==`.

* Ensure nested parenthesis are handled into links *Robin Dupret*

* Ensure nested code spans put in emphasis work correctly *Robin Dupret*

## Version 2.3.0

* Add a `:disable_indented_code_blocks` option *Dmitriy Kiriyenko*

* Fix issue [#57](https://github.com/vmg/redcarpet/issues/57) *Mike Morearty*

* Ensure new lines characters are inserted when using the StripDown
render. *Robin Dupret*

* Mark all symbols as hidden except the main entry point *Tom Hughes*

  This avoids conflicts with other gems that may have some of the
  same symbols, such as escape_utils which also uses houdini.

* Remove unnecessary function pointer *Sam Soffes*

* Add optional underline support *Sam Soffes*

  This is `*italic*` and this is `_underline_` when enabled.

* Test that links with quotes work *Michael Grosser*

* Adding a prettyprint class for google-code-prettify *Joel Rosenberg*

* Remove unused C macros *Matt Rogers*

* Remove 'extern' definition for Init_redcarpet_rndr() *Matt Rogers*

* Remove Gemfile.lock from the gemspec *Matt Rogers*

* Removed extra unused test statement. *Slipp D. Thompson*

* Use test-unit gem to get some red/green output when running tests
*Michael Grosser*

* Remove a deprecation warning and update Gemfile.lock *Robin Dupret*

* Added contributing file *Brent Beer*

* For tests for libxml2 > 2.8 *strzibny*

* SmartyPants: Preserve single `backticks` in HTML *Mike Morearty*

  When SmartyPants is processing HTML, single `backticks` should  be left
  intact. Previously they were being deleted.

* Removed and ignored Gemfile.lock *Ryan McGeary*

* Added support for org-table syntax *Ryan McGeary*

  Adds support for using a plus (+) as an intersection character instead of
  requiring pipes (|). The emacs org-mode table syntax automatically manages
  ascii tables, but uses pluses for line intersections.

* Ignore /tmp directory *Ryan McGeary*

* Add redcarpet_ prefix for `stack_*` functions *Kenta Murata*

* Mark any html_attributes has held by a renderer as used *Tom Hughes*

* Add Rubinius to the list of tested implementations *Gibheer*

* Add a changelog file
