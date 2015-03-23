require 'rubygems/xcodeproj_generator/rake_task'

Rubygems::XcodeprojGenerator::RakeTask.new do |project|
  project.build_command = 'bundle exec rake compile'
end
