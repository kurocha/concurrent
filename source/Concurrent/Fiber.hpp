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

#include "Condition.hpp"

#include "coro.h"

#include <string>
#include <list>

namespace Concurrent
{
	enum class Status
	{
		MAIN,
		READY,
		RUNNING,
		STOPPED,
		FINISHING,
		FINISHED
	};
	
	class Stop {};
	
	class Fiber
	{
	public:
		thread_local static Fiber main;
		thread_local static Fiber * current;
		thread_local static std::size_t level;
		
		static constexpr std::size_t DEFAULT_STACK_SIZE = 1024*16;
		
		Fiber(const std::string & annotation, const std::function<void()> & function, std::size_t stack_size = DEFAULT_STACK_SIZE) noexcept;
		Fiber(const std::function<void()> & function, std::size_t stack_size = DEFAULT_STACK_SIZE) noexcept;
		~Fiber();
		
		Fiber(const Fiber & other) = delete;
		Fiber & operator=(const Fiber & other) = delete;
		
		const Status & status() noexcept {return _status;}
		
		/// Resume the function.
		void resume();
		
		/// Yield back to the caller.
		void yield();

		/// Transfer control to this fiber.
		void transfer();

		/// Resumes the fiber, raising the Stop exception.
		void stop();

		/// Next time the fiber is resumed, it will be stopped?
		void cancel()
		{
			_status = Status::STOPPED;
		}
		
		/// Yield the calling fiber until this fiber completes execution.
		void wait();
		
		void annotate(const std::string & annotation) {_annotation = annotation;}
		
	private:
		class Stack : public coro_stack
		{
		public:
			Stack(std::size_t size);
			Stack();
			~Stack();
		};
		
		class Context : public coro_context
		{
		public:
			typedef void(*EntryT)(void*);
			
			Context(Stack & stack, EntryT entry, void * argument);
			Context();
			~Context();
		};
		
		Fiber() noexcept;
		[[noreturn]] static void coentry(void * arg);
		
		Status _status = Status::READY;
		std::string _annotation;
		
		std::function<void()> _function;
		
		Stack _stack;
		Context _context;
		
		std::exception_ptr _exception;
		
		Condition _completion;
		Fiber * _caller = nullptr;
		
	public:
		class Pool
		{
		public:
			Pool(std::size_t stack_size = DEFAULT_STACK_SIZE);
			~Pool();
			
			Fiber & resume(const std::function<void()> & function);
			
		protected:
			std::size_t _stack_size = 0;
			
			std::list<Stack> _stacks;
			std::list<Fiber> _fibers;
		};
	};
}
