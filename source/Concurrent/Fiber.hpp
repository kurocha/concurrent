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

#include "Stack.hpp"
#include "Condition.hpp"
#include "Coentry.hpp"

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
		
		// TODO assess how much of a performance impact this has in the presence of virtual memory. Can it be bigger? Should it be smaller?
		static constexpr std::size_t DEFAULT_STACK_SIZE = 1024*1024*4;
		
		template <typename FunctionT>
		Fiber(FunctionT && function, std::size_t stack_size = DEFAULT_STACK_SIZE) : _stack(stack_size), _context(_stack, function)
		{
		}
		
		template <typename FunctionT>
		Fiber(std::string annotation, FunctionT && function, std::size_t stack_size = DEFAULT_STACK_SIZE) : _annotation(annotation), _stack(stack_size), _context(_stack, function)
		{
		}
		
		~Fiber();
		
		Fiber(const Fiber & other) = delete;
		Fiber & operator=(const Fiber & other) = delete;
		
		const Status & status() const noexcept {return _status;}
		explicit operator bool() const noexcept {return _status != Status::FINISHED;}
		
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
		class Context : public coro_context
		{
		public:
			Context();
			
			template <typename FunctionT>
			Context(Stack & stack, FunctionT && function)
			{
				auto coentry = emplace_coentry(stack, std::move(function));
				
				coro_create(this, coentry->cocall, coentry, stack.base(), stack.size());
			}
			
			~Context();
		};
		
		Fiber() noexcept;
		[[noreturn]] static void coentry(void * arg);
		
		[[noreturn]] void coreturn();

#if defined(VARIANT_SANITIZE)
		void * _fake_stack = nullptr;
		void * _from_stack_bottom = nullptr;
		std::size_t _from_stack_size = 0;
		
		void start_push_stack(std::string annotation);
		void finish_push_stack(std::string annotation);
		
		void start_pop_stack(std::string annotation);
		void finish_pop_stack(std::string annotation, bool fake_stack = true);
#endif
		
		Status _status = Status::READY;
		std::string _annotation;
		
		Stack _stack;
		Context _context;
		
		std::exception_ptr _exception;
		
		Condition _completion;
		Fiber * _caller = nullptr;
		
		template <typename>
		friend struct Coentry;
		
	public:
		class Pool
		{
		public:
			Pool(std::size_t stack_size = DEFAULT_STACK_SIZE);
			~Pool();
			
			template <typename FunctionT>
			Fiber & resume(FunctionT && function)
			{
				_fibers.emplace_back(function);
				
				auto & fiber = _fibers.back();
					
				fiber.resume();
					
				return fiber;
			}
			
		protected:
			std::size_t _stack_size = 0;
			
			std::list<Stack> _stacks;
			std::list<Fiber> _fibers;
		};
	};
	
	template <typename FunctionT>
	[[noreturn]] void Coentry<FunctionT>::cocall(void * arg)
	{
		auto fiber = Fiber::current;
		auto * coentry = reinterpret_cast<Coentry*>(arg);
		
#if defined(VARIANT_SANITIZE)
		fiber->finish_push_stack("cocall");
#endif
		
		try {
			fiber->_status = Status::RUNNING;
			coentry->function();
		} catch (Stop) {
			// Ignore - not an actual error.
		} catch (...) {
			fiber->_exception = std::current_exception();
		}

		fiber->_status = Status::FINISHING;
		// Notify other fibers that we've completed.
		fiber->_completion.resume();
		fiber->_status = Status::FINISHED;
		
		// Going out of scope.
		coentry->~Coentry();
		
		fiber->coreturn();
	}
}
