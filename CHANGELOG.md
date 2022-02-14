# Changelog

## Unreleased

* Bump nokogiri from 1.6.0 to 1.11.7
  * Support cpu archtecture for arm64/aarch64 systems (like Apple's M1)
* Change ci platform to Github Actions

## v3.5.1.2

* Support custom block notation.
  * It starts with `:::` and ends with `:::`.
  * Output a `<div data-type="customblock" data-metadata="">` element.
  * Passes the string following `:::` to the `data-metadata` attribute.

## v3.5.1.1

* Unsupport details and summary tags.

## v3.5.1.0

* Update base Redcarpet version to 3.5.1.

## v3.2.2.4

* Relax max nesting.

## v3.2.2.3

* Change `<code>` attribute for code block metadata (language) from `class` to `data-metadata`.
  Note that this is a breaking change, though you won't face this breakage if you're using greenmat through qiita-markdown gem and updating both gems.

## v3.2.2.2

* Fix bugs in UTF-8 handling. [#3](https://github.com/increments/greenmat/pull/3) ([@gfx](https://github.com/gfx))

## v3.2.2.1

* Fix a bug where bad memory access would happen in a document starting with `@`.

## v3.2.2.0

* Update base Redcarpet version to 3.2.2.

## v3.2.0.2

* Fix missing `greenmat/version` in the gem package.

## v3.2.0.1

* Add `no_mention_emphasis` option to disable emphasizing mentions.

## v3.2.0.0

* Initial release.
