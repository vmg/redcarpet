namespace :greenmat do
  desc 'Rename Redcarpet project to Greenmat'
  task :rename_project do
    ProjectRenamer.rename
  end

  desc 'Set up git remote for redcarpet as `upstream`.'
  task :setup_upstream do
    sh 'git remote add upstream https://github.com/vmg/redcarpet.git'
    sh 'git remote update'
  end

  desc 'Merge upstream branch while renaming project (BRANCH=branch_name).'
  task :merge_upstream do
    abort 'The current repository is not clean.' unless `git status --porcelain`.empty?

    main_branch = `git rev-parse --abbrev-ref HEAD`.chomp
    target_branch = ENV['BRANCH'] || 'upstream/master'
    target_merge_branch = "merge-#{target_branch}"

    puts "Merging #{target_branch.inspect} into #{main_branch.inspect}"

    require 'fileutils'
    Dir.mkdir('tmp') unless Dir.exist?('tmp')
    FileUtils.cp('tasks/greenmat.rake', 'tmp')

    sh 'git', 'checkout', '-B', target_merge_branch, target_branch
    sh 'rake --rakefile tmp/greenmat.rake greenmat:rename_project'
    sh 'git', 'add', '.'
    sh 'git', 'commit', '--message', 'Rename project'

    sh 'git', 'checkout', main_branch
    sh 'git', 'merge', '--no-commit', '--strategy-option', 'theirs', target_merge_branch
  end
end

module ProjectRenamer
  PATH_MAP = {
    'redcarpet' => 'greenmat',
    'rc_'       => 'gm_'
  }

  SYMBOL_MAP = {
    'redcarpet' => 'greenmat',
    'Redcarpet' => 'Greenmat',
    'REDCARPET' => 'GREENMAT',
    /\brc_/     => 'gm_'
  }

  SYMBOL_RENAME_EXCLUSION_PATH_PATTERNS = [
    /^tasks\//,
    /^tmp\//,
    /\.(?:bundle|so)$/,
    /README/,
    /CHANGELOG.md/,
    /CONTRIBUTING\.md/
  ]

  module_function

  def rename
    rename_paths
    rename_symbols
  end

  def rename_paths
    Dir['**/*'].each do |path|
      PATH_MAP.each do |old, new|
        next unless path.include?(old)
        is_directory = File.directory?(path)
        File.rename(path, path.gsub(old, new))
        fail RenamedDirectory if is_directory
      end
    end
  rescue RenamedDirectory
    retry
  end

  def rename_symbols
    Dir['**/*'].each do |path|
      next unless File.file?(path)
      next if SYMBOL_RENAME_EXCLUSION_PATH_PATTERNS.any? { |pattern| path.match(pattern) }

      source = File.read(path)

      SYMBOL_MAP.each do |old, new|
        source.gsub!(old, new)
      end

      File.write(path, source)
    end
  end

  class RenamedDirectory < StandardError
  end
end
