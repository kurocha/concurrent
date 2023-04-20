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

#if defined(CONCURRENT_SANITIZE_ADDRESS)
#include <sanitizer/common_interface_defs.h>
#endif

namespace Concurrent
{
	thread_local Fiber Fiber::main;
	thread_local Fiber * Fiber::current = &Fiber::main;
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
	
#if defined(CONCURRENT_SANITIZE_ADDRESS)
	void Fiber::start_push_stack(std::string annotation)
	{
		// std::cerr << "Fiber::start_push_stack(" << annotation << ", " << _stack.base() << ", " << _stack.allocated_size() << ")" << std::endl;
		__sanitizer_start_switch_fiber(&_fake_stack, _stack.base(), _stack.allocated_size());
	}
	
	void Fiber::finish_push_stack(std::string annotation)
	{
		__sanitizer_finish_switch_fiber(_fake_stack, &_from_stack_bottom, &_from_stack_size);
		// std::cerr << "Fiber::finish_push_stack(" << annotation << ", " << _from_stack_bottom << ", " << _from_stack_size << ")" << std::endl;
	}
	
	void Fiber::start_pop_stack(std::string annotation, bool terminating)
	{
		// std::cerr << "Fiber::start_pop_stack(" << annotation << ", " << _from_stack_bottom << ", " << _from_stack_size << ", " << terminating << ")" << std::endl;
		__sanitizer_start_switch_fiber(terminating ? nullptr : &_fake_stack, _from_stack_bottom, _from_stack_size);
	}
	
	void Fiber::finish_pop_stack(std::string annotation)
	{
		__sanitizer_finish_switch_fiber(_fake_stack, &_from_stack_bottom, &_from_stack_size);
		// std::cerr << "Fiber::finish_pop_stack(" << annotation << ", " << _from_stack_bottom << ", " << _from_stack_size << ")" << std::endl;
	}
#endif
	
	void Fiber::resume()
	{
		// We cannot double-resume.
		assert(_caller == nullptr);

		_caller = Fiber::current;

		assert(_status != Status::FINISHED);

		Fiber::current = this;
		// std::cerr << std::string(Fiber::level, '\t') << _caller->_annotation << " resuming " << _annotation << std::endl;

		Fiber::level += 1;

#if defined(CONCURRENT_SANITIZE_ADDRESS)
		start_push_stack("resume");
#endif
	
		// Switch from the fiber that called this function to the fiber this object represents.
		coroutine_transfer(&_caller->_context, &_context);

#if defined(CONCURRENT_SANITIZE_ADDRESS)
		finish_pop_stack("resume");
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

#if defined(CONCURRENT_SANITIZE_ADDRESS)
		start_pop_stack("yield");
#endif

		coroutine_transfer(&_context, &_caller->_context);

#if defined(CONCURRENT_SANITIZE_ADDRESS)
		finish_push_stack("yield");
#endif

		// std::cerr << std::string(Fiber::level, '\t') << "yield back to " << _annotation << std::endl;

		if (_status == Status::STOPPED) {
			throw Stop();
		}
	}

	void Fiber::transfer()
	{
		Fiber * current = Fiber::current;

		// Transferring to ourselves is a no-op.
		if (current == this) return;

		// std::cerr << std::string(Fiber::level, '\t') << "transfer from " << current->_annotation << " to " << _annotation << std::endl;

#if defined(CONCURRENT_SANITIZE_ADDRESS)
		start_push_stack("transfer");
#endif

		Fiber::current = this;

		coroutine_transfer(&current->_context, &_context);

		Fiber::current = current;

#if defined(CONCURRENT_SANITIZE_ADDRESS)
		finish_pop_stack("transfer");
#endif
	}
	
	void Fiber::coreturn()
	{
		assert(_caller != nullptr);

		// std::cerr << std::string(Fiber::level, '\t') << _annotation << " terminating to " << _caller->_annotation << std::endl;

#if defined(CONCURRENT_SANITIZE_ADDRESS)
		start_pop_stack("coreturn", true);
#endif

		coroutine_transfer(&_context, &_caller->_context);
		
		std::terminate();
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
		this->stack_pointer = nullptr;
	}
	
	Fiber::Context::~Context()
	{
		this->stack_pointer = nullptr;
	}
	
	Fiber::Pool::Pool(std::size_t stack_size) : _stack_size(stack_size)
	{
	}
	
	Fiber::Pool::~Pool()
	{
		// std::cerr << "Fiber pool going out of scope with " << _fibers.size() << " fibers allocated" << std::endl;
		// 
		// for (auto && fiber : _fibers) {
		// 	std::cerr << "\tFiber " << &fiber << " stack " << fiber.stack().top() << ": " << fiber.annotation() << " (" << (std::size_t)(fiber.status()) << ")" << std::endl;
		// }
	}
}
