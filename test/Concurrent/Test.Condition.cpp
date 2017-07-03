//
//  Test.Condition.cpp
//  This file is part of the "Concurrent" project and released under the MIT License.
//
//  Created by Samuel Williams on 29/6/2017.
//  Copyright, 2017, by Samuel Williams. All rights reserved.
//

#include <UnitTest/UnitTest.hpp>

#include <Concurrent/Condition.hpp>
#include <Concurrent/Fiber.hpp>

namespace Concurrent
{
	UnitTest::Suite ConditionTestSuite {
		"Concurrent::Condition",
		
		{"it should wait until signalled",
			[](UnitTest::Examiner & examiner) {
				Condition condition;
				
				Fiber fiber([&]{
					condition.wait();
				});
				
				examiner.expect(condition.count()) == 0;
				
				fiber.resume();
				
				examiner.expect(condition.count()) == 1;
				
				condition.resume();
				
				examiner.expect(condition.count()) == 0;
			}
		},
	};
}
