require 'rspec/core/rake_task'

RSpec::Core::RakeTask.new(:spec) do |task|
  task.verbose = false
end
