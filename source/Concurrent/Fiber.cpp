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
	thread_local std::size_t Fiber::level = 0;
	
	Fiber::Fiber() noexcept : _status(Status::MAIN), _annotation("main")
	{
	}
	
	Fiber::Fiber(const std::string & annotation, std::function<void()> && function, std::size_t stack_size) noexcept : _annotation(annotation), _function(std::move(function)), _stack(stack_size), _context(_stack, &coentry, this)
	{
	}
	
	Fiber::Fiber(std::function<void()> && function, std::size_t stack_size) noexcept : Fiber("", std::move(function), stack_size)
	{
	}
	
	Fiber::~Fiber()
	{
		// std::cerr << std::string(Fiber::level, '\t') << "-> ~Fiber " << _annotation << std::endl;

		if (_status == Status::READY) {
			// Nothing to do here.
		} else if (_status == Status::RUNNING) {
			// Force fiber to stop.
			stop();
		} else if (_status == Status::FINISHING) {
			// Still cleaning up...
			resume();
		}
		
		// std::cerr << std::string(Fiber::level, '\t') << "<- ~Fiber " << _annotation << std::endl;
	}
	
	void Fiber::coentry(void * arg)
	{
		auto fiber = reinterpret_cast<Fiber *>(arg);
		
		try {
			fiber->_status = Status::RUNNING;
			fiber->_function();
		} catch (Stop) {
			// Ignore - not an actual error.
		} catch (...) {
			fiber->_exception = std::current_exception();
		}

		fiber->_status = Status::FINISHING;

		// std::cerr << std::string(Fiber::level, '\t') << "*** coroutine completion " << fiber->_annotation << std::endl;

		// Notify other fibers that we've completed.
		fiber->_completion.resume();

		fiber->_status = Status::FINISHED;

		// std::cerr << std::string(Fiber::level, '\t') << "*** coroutine yield " << fiber->_annotation << std::endl;

		fiber->yield();
		
		std::terminate();
	}
	
	void Fiber::resume()
	{
		// We cannot double-resume.
		assert(_caller == nullptr);

		if (Fiber::current) {
			_caller = Fiber::current;
		} else {
			_caller = &Fiber::main;
		}

		assert(_status != Status::FINISHED);

		Fiber::current = this;
		// std::cerr << std::string(Fiber::level, '\t') << _caller->_annotation << " resuming " << _annotation << std::endl;

		Fiber::level += 1;
		coro_transfer(&_caller->_context, &_context);
		Fiber::level -= 1;

		// std::cerr << std::string(Fiber::level, '\t') << "resume back in " << _caller->_annotation << std::endl;

		Fiber::current = _caller;

		this->_caller = nullptr;

		// Once we yield back to the caller, if there was an exception, we rethrow it.
		if (_exception) {
			std::rethrow_exception(_exception);
		}
	}

	void Fiber::yield()
	{
		assert(_caller != nullptr);

		// std::cerr << std::string(Fiber::level, '\t') << _annotation << " yielding to " << _caller->_annotation << std::endl;

		coro_transfer(&_context, &_caller->_context);

		// std::cerr << std::string(Fiber::level, '\t') << "yield back to " << _annotation << std::endl;

		if (_status == Status::STOPPED) {
			throw Stop();
		}
	}

	void Fiber::transfer()
	{
		Fiber * current = Fiber::current;

		if (current == nullptr) {
			current = &Fiber::main;
		}

		// std::cerr << std::string(Fiber::level, '\t') << "transfer from " << current->_annotation << " to " << _annotation << std::endl;

		coro_transfer(&current->_context, &_context);
	}
	
	void Fiber::wait()
	{
		// Cannot wait for own self to complete.
		assert(Fiber::current != this);
		
		_completion.wait();
	}
	
	void Fiber::stop()
	{
		// Cannot stop self.
		assert(Fiber::current != this);
		
		_status = Status::STOPPED;
		
		resume();
	}
	
	Fiber::Stack::Stack(std::size_t size)
	{
		coro_stack_alloc(this, size);
	}
	
	Fiber::Stack::Stack()
	{
		sptr = nullptr;
		ssze = 0;
	}
	
	Fiber::Stack::~Stack()
	{
		if (sptr) {
			coro_stack_free(this);
		}
	}
	
	Fiber::Context::Context()
	{
		coro_create(this, nullptr, nullptr, nullptr, 0);
	}
	
	Fiber::Context::Context(Stack & stack, EntryT entry, void * argument)
	{
		coro_create(this, entry, argument, stack.sptr, stack.ssze);
	}
	
	Fiber::Context::~Context()
	{
		coro_destroy(this);
	}
	
	Fiber::Pool::Pool(std::size_t stack_size) : _stack_size(stack_size)
	{
	}
	
	Fiber::Pool::~Pool()
	{
	}
	
	Fiber & Fiber::Pool::resume(std::function<void()> && function)
	{
		// if (_stack.empty()) {
			// _fibers.emplace_back(function)
		// } else {
			// auto stack = _stacks.back();
			
			_fibers.emplace_back(std::move(function)/*, stack, this*/);
			// _stacks.pop_back();
			
			auto & fiber = _fibers.back();
			
			fiber.resume();
			
			return fiber;
		// }
	}
}
