namespace :greenmat do
  desc 'Rename Redcarpet project to Greenmat'
  task :rename_project do
    ProjectRenamer.rename
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
    /\.(?:bundle|so)$/
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
