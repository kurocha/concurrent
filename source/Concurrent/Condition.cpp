//
//  Condition.cpp
//  File file is part of the "Concurrent" project and released under the MIT License.
//
//  Created by Samuel Williams on 29/6/2017.
//  Copyright, 2017, by Samuel Williams. All rights reserved.
//

#include "Condition.hpp"

#include "Fiber.hpp"

#include <iostream>

namespace Concurrent
{
	Condition::Condition()
	{
	}
	
	Condition::~Condition()
	{
		// std::cerr << "Condition@" << this << "::~Condition _waiting=" << _waiting.size() << " _current=" << Fiber::current << std::endl;
		while (!_waiting.empty()) {
			auto fiber = _waiting.back();
			_waiting.pop_back();

			if (fiber)
				fiber->stop();
		}
	}
	
	void Condition::wait()
	{
		// std::cerr << "Condition@" << this << "::wait _current=" << Fiber::current << std::endl;

		_waiting.push_back(Fiber::current);
		Fiber::current->yield();
	}
	
	void Condition::resume()
	{
		while (!_waiting.empty()) {
			auto fiber = _waiting.back();
			_waiting.pop_back();

			if (fiber->status() != Status::FINISHED)
				fiber->resume();
		}
	}
}
