//
//  Stack.cpp
//  File file is part of the "Concurrent" project and released under the MIT License.
//
//  Created by Samuel Williams on 4/7/2017.
//  Copyright, 2017, by Samuel Williams. All rights reserved.
//

#include "Stack.hpp"

#include <unistd.h>
#include <stddef.h>

#include <sys/mman.h>

namespace Concurrent
{
	const std::size_t Stack::ALIGNMENT = 16;
	
	Stack::Stack(std::size_t size)
	{
		const std::size_t GUARD_PAGES = 1;
		static const std::size_t PAGE_SIZE = sysconf(_SC_PAGESIZE);
		
		std::size_t page_count = ((size+PAGE_SIZE) / PAGE_SIZE);
		auto stack_size = (GUARD_PAGES + page_count) * PAGE_SIZE;
		
		// The base allocation:
		_base = ::mmap(0, stack_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		
		if (_base == MAP_FAILED) {
			_base = nullptr;
			
			throw std::system_error(errno, std::generic_category(), "mmap(...)");
		}
		
		// The top of the stack:
		_top = (Byte*)_base + stack_size;
		
		// The current top of the stack, taking into account any emplacements:
		_current = _top;
		
		// Protect the bottom of the stack so we don't have silent stack overflow:
		::mprotect(_base, GUARD_PAGES*PAGE_SIZE, PROT_NONE);
		_bottom = (Byte*)_base + GUARD_PAGES*PAGE_SIZE;
	}
	
	Stack::Stack() : _base(nullptr), _bottom(nullptr), _current(nullptr), _top(nullptr)
	{
	}
	
	Stack::~Stack() noexcept(false)
	{
		if (_base) {
			auto result = ::munmap(_base, (Byte*)_top - (Byte*)_base);
			
			if (result == -1) {
				throw std::system_error(errno, std::generic_category(), "munmap(...)");
			}
		}
	}
	
	Stack::Stack(Stack && other)
	{
		_base = other._base;
		_bottom = other._bottom;
		_current = other._current;
		_top = other._top;
		
		other._base = nullptr;
		other._bottom = nullptr;
		other._current = nullptr;
		other._top = nullptr;
	}
	
	Stack & Stack::operator=(Stack && other)
	{
		if (_base) {
			::munmap(_base, (Byte*)_top - (Byte*)_base);
		}
		
		_base = other._base;
		_bottom = other._bottom;
		_current = other._current;
		_top = other._top;
		
		other._base = nullptr;
		other._bottom = nullptr;
		other._current = nullptr;
		other._top = nullptr;
		
		return *this;
	}
}
