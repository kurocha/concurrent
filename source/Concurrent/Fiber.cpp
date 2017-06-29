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

namespace Concurrent
{
	thread_local Fiber Fiber::main;
	thread_local Fiber * Fiber::current;
	
	Fiber::Fiber() noexcept
	{
		coro_create(&_context, nullptr, nullptr, nullptr, 0);
	}
	
	Fiber::Fiber(std::function<void()> function, std::size_t stack_size) noexcept : _function(function)
	{
		coro_stack_alloc(&_stack, stack_size);
		coro_create(&_context, &coentry, static_cast<void *>(this), _stack.sptr, _stack.ssze);
	}
	
	Fiber::~Fiber()
	{
		if (_status == Status::RUNNING) {
			throw std::logic_error("fiber still running");
		}
		
		if (this != &main) {
			coro_stack_free(&_stack);
			coro_destroy(&_context);
		}
	}
	
	void Fiber::coentry(void * arg)
	{
		auto current = reinterpret_cast<Fiber *>(arg);
		
		current->_status = Status::RUNNING;
		
		try {
			current->_function();
			current->_status = Status::FINISHED;
		} catch (Stop) {
			// Ignore - not an actual error.
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
		coro_transfer(&_caller->_context, &_context);
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
		
		coro_transfer(&_context, &_caller->_context);
		
		if (_status == Status::STOPPED) {
			throw Stop();
		}
	}
}
