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

#if defined(VARIANT_SANITIZE)
#include <sanitizer/common_interface_defs.h>
#endif

namespace Concurrent
{
	thread_local Fiber Fiber::main;
	thread_local Fiber * Fiber::current;
	thread_local std::size_t Fiber::level = 0;
	
	Fiber::Fiber() noexcept : _status(Status::MAIN), _annotation("main")
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

#if defined(VARIANT_SANITIZE)
		void * fake_stack = nullptr;
		__sanitizer_start_switch_fiber(&fake_stack, _stack.base(), _stack.allocated_size());
		// std::cerr << "__sanitizer_start_switch_fiber (resume, fake_stack=" << fake_stack << ")" << std::endl;
#endif

		coro_transfer(&_caller->_context, &_context);

#if defined(VARIANT_SANITIZE)
		// std::cerr << "__sanitizer_finish_switch_fiber (resume, fake_stack=" << fake_stack << ")" << std::endl;
		__sanitizer_finish_switch_fiber(fake_stack, nullptr, nullptr);
#endif

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

#if defined(VARIANT_SANITIZE)
		void * fake_stack = nullptr;
		__sanitizer_start_switch_fiber(&fake_stack, _caller->_stack.base(), _caller->_stack.allocated_size());
		// std::cerr << "__sanitizer_start_switch_fiber (yield, fake_stack=" << fake_stack << ")" << std::endl;
#endif

		coro_transfer(&_context, &_caller->_context);

#if defined(VARIANT_SANITIZE)
		// std::cerr << "__sanitizer_finish_switch_fiber (yield, fake_stack=" << fake_stack << ")" << std::endl;
		__sanitizer_finish_switch_fiber(fake_stack, nullptr, nullptr);
#endif

		// std::cerr << std::string(Fiber::level, '\t') << "yield back to " << _annotation << std::endl;

		if (_status == Status::STOPPED) {
			throw Stop();
		}
	}
	
	void Fiber::coreturn()
	{
		assert(_caller != nullptr);

		// std::cerr << std::string(Fiber::level, '\t') << _annotation << " terminating to " << _caller->_annotation << std::endl;

#if defined(VARIANT_SANITIZE)
		// std::cerr << "__sanitizer_start_switch_fiber (terminate, fake_stack=nullptr)" << std::endl;
		__sanitizer_start_switch_fiber(nullptr, _caller->_stack.base(), _caller->_stack.allocated_size());
#endif

		coro_transfer(&_context, &_caller->_context);
		
		std::terminate();
	}

	void Fiber::transfer()
	{
		Fiber * current = Fiber::current;

		if (current == nullptr) {
			current = &Fiber::main;
		}

		// std::cerr << std::string(Fiber::level, '\t') << "transfer from " << current->_annotation << " to " << _annotation << std::endl;

#if defined(VARIANT_SANITIZE)
		void * fake_stack = nullptr;
		__sanitizer_start_switch_fiber(&fake_stack, _stack.base(), _stack.allocated_size());
		// std::cerr << "__sanitizer_start_switch_fiber (transfer, fake_stack=" << fake_stack << ")" << std::endl;
#endif

		coro_transfer(&current->_context, &_context);

#if defined(VARIANT_SANITIZE)
		// std::cerr << "__sanitizer_finish_switch_fiber (transfer, fake_stack=" << fake_stack << ")" << std::endl;
		__sanitizer_finish_switch_fiber(fake_stack, nullptr, nullptr);
#endif
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
	
	Fiber::Context::Context()
	{
		coro_create(this, nullptr, nullptr, nullptr, 0);
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
}
