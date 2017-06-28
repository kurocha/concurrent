//
//  Fiber.hpp
//  File file is part of the "Memory" project and released under the MIT License.
//
//  Created by Samuel Williams on 28/6/2017.
//  Copyright, 2017, by Samuel Williams. All rights reserved.
//

#pragma once

#include <functional>
#include <exception>

namespace Concurrent
{
	enum class Status
	{
		READY,
		RUNNING,
		STOPPED,
		FAILED,
		FINISHED
	};
	
	class Stop {};
	
	class Fiber
	{
	public:
		thread_local static Fiber main;
		thread_local static Fiber * current;
		
		static void yield() {current->yield();}
		
		Fiber(std::function<void()> function, std::size_t stack_size = 1024*4) noexcept;
		~Fiber();
		
		const Status & status() noexcept {return _status;}
		
		/// Resume the function.
		void resume();
		
		/// Yield back to the caller.
		void yield();
		
		/// Resumes the fiber, raising the Stop exception.
		void stop();
		
	private:
		Fiber();
		[[noreturn]] static void coentry();
		
		void * _context = nullptr;
		Status _status = Status::READY;
		
		Fiber * _caller = nullptr;
		std::exception_ptr _exception;
		
		std::function<void()> _function;
	};
}
