
#
#  This file is part of the "Teapot" project, and is released under the MIT license.
#

teapot_version "1.3"

define_project "Concurrent" do |project|
	project.add_author "Samuel Williams"
	project.license = "MIT License"

	project.version = "1.0.0"
end

define_target "concurrent" do |target|
	target.build do |environment|
		source_root = target.package.path + 'source'
		
		copy headers: source_root.glob('Concurrent/**/*.{hpp,h}')
		
		build static_library: "Concurrent", source_files: source_root.glob('Concurrent/**/*.{cpp,c}')
	end
	
	target.depends :platform
	target.depends "Language/C++11", private: true
	
	target.depends "Build/Files"
	target.depends "Build/Clang"
	
	target.provides "Library/Concurrent" do
		append linkflags ->{install_prefix + "lib/libConcurrent.a"}
	end
end

define_target "concurrent-tests" do |target|
	target.build do |environment|
		test_root = target.package.path + 'test'
		
		run tests: "Concurrent", source_files: test_root.glob('Concurrent/**/*.cpp')
	end
	
	target.depends "Language/C++11", private: true
	
	target.depends "Library/UnitTest"
	target.depends "Library/Concurrent"
	
	target.provides "Test/Concurrent"
end

define_configuration "test" do |configuration|
	configuration[:source] = "http://github.com/kurocha/"
	
	configuration.require "platforms"
	configuration.require "build-files"
	
	configuration.require "unit-test"
	
	configuration.require "language-cpp-class"
end
