
#
#  This file is part of the "Teapot" project, and is released under the MIT license.
#

teapot_version "1.3"

# Project Metadata

define_project "concurrent" do |project|
	project.title = "Concurrent"
	
	project.summary = 'Primitives for concurrent execution.'
	
	project.license = "MIT License"
	
	project.add_author 'Samuel Williams', email: 'samuel.williams@oriontransfer.co.nz'
	
	project.version = "1.0.0"
end

# Build Targets

define_target 'concurrent-library' do |target|
	target.depends "Language/C++14"
	
	target.depends "Library/Coroutine", public: true
	
	target.provides "Library/Concurrent" do
		source_root = target.package.path + 'source'
		
		library_path = build static_library: "Concurrent", source_files: source_root.glob('Concurrent/**/*.{cpp,c}')
		
		append linkflags library_path
		append header_search_paths source_root
	end
end

define_target "concurrent-tests" do |target|
	target.depends "Language/C++14"
	
	target.depends "Library/UnitTest"
	target.depends "Library/Concurrent"
	
	target.provides "Test/Concurrent" do |*arguments|
		test_root = target.package.path
		
		run tests: 'Concurrent', source_files: test_root.glob('test/Concurrent/**/*.cpp'), arguments: arguments
	end
end

# Configurations

define_configuration "development" do |configuration|
	configuration[:source] = "https://github.com/kurocha/"
	
	configuration.import "concurrent"
	
	# Provides all the build related infrastructure:
	configuration.require "platforms"
	configuration.require "build-files"
	
	# Provides unit testing infrastructure and generators:
	configuration.require "unit-test"
	
	# Provides some useful C++ generators:
	configuration.require 'generate-cpp-class'
	configuration.require 'generate-project'
	configuration.require 'generate-travis'
end

define_configuration "concurrent" do |configuration|
	configuration.public!
	
	configuration.require "coroutine"
	
	# Provides all the build related infrastructure:
	configuration.require "platforms"
	configuration.require "build-files"
	
	# Provides unit testing infrastructure and generators:
	configuration.require "unit-test"
	
	# Provides some useful C++ generators:
	configuration.require 'generate-cpp-class'
	configuration.require 'generate-project'
	configuration.require 'generate-travis'
end
