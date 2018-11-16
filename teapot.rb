
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
	target.build do
		source_root = target.package.path + 'source'
		
		copy headers: source_root.glob('Concurrent/**/*.{hpp,h}')
		
		build static_library: "Concurrent", source_files: source_root.glob('Concurrent/**/*.{cpp,c}')
	end
	
	target.depends 'Build/Files'
	target.depends 'Build/Clang'
	
	target.depends :platform
	target.depends "Language/C++11", private: true
	
	target.depends "Library/Coroutine"
	
	target.provides "Library/Concurrent" do
		append linkflags [
			->{install_prefix + 'lib/libConcurrent.a'},
		]
	end
end

define_target "concurrent-tests" do |target|
	target.build do |*arguments|
		run tests: 'Concurrent', source_files: target.package.path.glob('test/Concurrent/**/*.cpp'), arguments: arguments
	end
	
	target.depends "Language/C++14", private: true
	
	target.depends "Library/UnitTest"
	target.depends "Library/Concurrent"
	
	target.provides "Test/Concurrent"
end

# Configurations

define_configuration "development" do |configuration|
	configuration[:source] = "http://github.com/kurocha/"
	
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

define_configuration "concurrent" do |configuration|
	configuration[:source] = "http://github.com/kurocha/"
	
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
