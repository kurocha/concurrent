//
//  Condition.hpp
//  File file is part of the "Concurrent" project and released under the MIT License.
//
//  Created by Samuel Williams on 29/6/2017.
//  Copyright, 2017, by Samuel Williams. All rights reserved.
//

#pragma once

#include "Fiber.hpp"

#include <vector>

namespace Concurrent
{
	// A synchronization primative, which allows fibers to wait until a particular condition is triggered.
	class Condition
	{
	public:
		Condition();
		virtual ~Condition();
		
		void wait()
		{
			_waiting.push_back(Fiber::current);
			Fiber::current->yield();
		}
		
		void signal()
		{
			while (!_waiting.empty()) {
				auto fiber = _waiting.back();
				_waiting.pop_back();
				
				fiber->resume();
			}
		}
		
		std::size_t count() const noexcept
		{
			return _waiting.size();
		}
		
	private:
		std::vector<Fiber *> _waiting;
	};
}
