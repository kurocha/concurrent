//
//  Condition.hpp
//  File file is part of the "Concurrent" project and released under the MIT License.
//
//  Created by Samuel Williams on 29/6/2017.
//  Copyright, 2017, by Samuel Williams. All rights reserved.
//

#pragma once

#include <vector>

namespace Concurrent
{
	class Fiber;
	
	// A synchronization primative, which allows fibers to wait until a particular condition is triggered.
	class Condition
	{
	public:
		Condition();
		
		// If a condition goes out of scope, all fibers waiting on it will be stopped.
		~Condition();
		
		Condition(const Condition & other) = delete;
		Condition & operator=(const Condition & other) = delete;
		
		void wait();
		void resume();
		
		std::size_t count() const noexcept {return _waiting.size();}
		
	private:
		std::vector<Fiber *> _waiting;
	};
}
