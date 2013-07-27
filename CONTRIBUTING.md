## Submitting a Pull Request

1. [Fork the repository.][fork]
2. [Create a topic branch.][branch]
3. Check which version of Ruby is installed on your machine with `ruby -v`.
   The list of supported Ruby versions is listed in [.travis.yml][travis_yml].
   Set up one of these versions; use of [RVM][rvm] is recommended to switch
   easily between different versions.
4. [Install bundler.][bundler]
5. Check that unit tests pass with `rake test`.
6. Write a failing test to capture existing bug or lack of feature.
7. Run `rake test` to verify that test fails.
8. Implement your feature or bug fix.
9. Ensure tests pass.
10. If it's a new feature or a bug fix,
    please add an entry to the changelog file.
11. Add, commit, and push your changes.
12. [Submit a pull request.][pr]
13. You will get some feedback and may need to push additional commits
    with more fixes to the same branch; this will update your pull request
    automatically.
14. [Squash your commits into a single one][rebase]
    and [force a push to the same branch][force_push].

[branch]: http://git-scm.com/book/en/Git-Branching-Branching-Workflows#Topic-Branches
[bundler]: http://bundler.io
[force_push]: https://help.github.com/articles/dealing-with-non-fast-forward-errors
[fork]: https://help.github.com/articles/fork-a-repo
[pr]: https://help.github.com/articles/using-pull-requests
[rebase]: https://help.github.com/articles/interactive-rebase
[rvm]: https://rvm.io
[travis_yml]: https://github.com/vmg/redcarpet/blob/master/.travis.yml
