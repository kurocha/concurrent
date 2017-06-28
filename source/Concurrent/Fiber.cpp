//
//  Fiber.cpp
//  File file is part of the "Memory" project and released under the MIT License.
//
//  Created by Samuel Williams on 28/6/2017.
//  Copyright, 2017, by Samuel Williams. All rights reserved.
//

#include "Fiber.hpp"

#include <stdexcept>
#include <iostream>
#include <cassert>

#include "Coroutine/libco.h"

namespace Concurrent
{
	thread_local Fiber Fiber::main;
	thread_local Fiber * Fiber::current;
	
	Fiber::Fiber()
	{
		_context = co_active();
	}
	
	Fiber::Fiber(std::function<void()> function, std::size_t stack_size) : _function(function)
	{
		_context = co_create(stack_size, &coentry);
	}
	
	Fiber::~Fiber()
	{
		if (_status == Status::RUNNING) {
			throw std::logic_error("fiber still running");
		}
		
		if (_context != main._context) {
			co_delete(_context);
		}
	}
	
	void Fiber::coentry()
	{
		auto current = Fiber::current;
		
		current->_status = Status::RUNNING;
		
		try {
			current->_function();
			current->_status = Status::FINISHED;
		} catch (Stop) {
			// Ignore.
		} catch (...) {
			current->_status = Status::FAILED;
			current->_exception = std::current_exception();
		}
		
		current->yield();
		
		throw std::logic_error("resume dead fiber");
	}
	
	void Fiber::resume()
	{
		assert(Fiber::current != this);
		
		if (Fiber::current) {
			_caller = Fiber::current;
		} else {
			_caller = &Fiber::main;
		}
		
		Fiber::current = this;
		co_switch(_context);
		Fiber::current = _caller;
		
		if (_exception) {
			std::rethrow_exception(_exception);
		}
	}
	
	void Fiber::stop()
	{
		assert(Fiber::current != this);
		
		_status = Status::STOPPED;
		
		resume();
	}
	
	void Fiber::yield()
	{
		assert(Fiber::current == this);
		
		co_switch(_caller->_context);
		
		if (_status == Status::STOPPED) {
			throw Stop();
		}
	}
}
