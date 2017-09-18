//
//  Stack.hpp
//  File file is part of the "Concurrent" project and released under the MIT License.
//
//  Created by Samuel Williams on 4/7/2017.
//  Copyright, 2017, by Samuel Williams. All rights reserved.
//

#pragma once

#include <memory>
#include <algorithm>

namespace Concurrent
{
	class Stack
	{
		typedef unsigned char Byte;
		
	public:
		Stack(std::size_t size);
		Stack();
		
		Stack(const Stack & other) = delete;
		Stack & operator=(const Stack & other) = delete;
		
		Stack(Stack && other);
		Stack & operator=(Stack && other);
		
		~Stack();
		
		static const std::size_t ALIGNMENT;
		
		template <typename Type, typename... Args>
		Type* emplace(Args&&... args)
		{
			auto alignment = std::max(ALIGNMENT, alignof(Type));
			
			// The stack grows from top towards bottom.
			void * next = (Byte*)_current - (sizeof(Type) + alignment);
			std::size_t space = (Byte*)_current - (Byte*)next;
			
			std::align(alignment, sizeof(Type), next, space);
			_current = next;
			
			return new(_current) Type(std::forward<Args>(args)...);
		}
		
		template <typename Type>
		Type * push(Type && value)
		{
			auto alignment = std::max(ALIGNMENT, alignof(Type));
			
			// The stack grows from top towards bottom.
			void * next = (Byte*)_current - (sizeof(Type) + alignof(Type));
			std::size_t space = (Byte*)_current - (Byte*)next;
			
			std::align(alignment, sizeof(Type), next, space);
			_current = next;
			
			return new(_current) Type(std::move(value));
		};
		
		// A pointer to the stack memory allocation.
		void * base() {return _base;}
		void * bottom() {return _bottom;}
		void * current() {return _current;}
		void * top() {return _top;}
		
		// The current available stack space:
		std::size_t size() {return (Byte*)_current - (Byte*)_bottom;}
		std::size_t allocated_size() {return (Byte*)top() - (Byte*)base();}
		
	private:
		void * _base, * _bottom, * _current, * _top;
	};
}
