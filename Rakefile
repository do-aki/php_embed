#!/usr/bin/env rake
require "bundler/gem_tasks"

require 'rspec'
require 'rspec/core/rake_task'

desc "Run test"
RSpec::Core::RakeTask.new('spec') do |t|
    t.verbose = true
end

task :default => :spec
