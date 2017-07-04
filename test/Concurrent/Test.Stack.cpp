//
//  Test.Stack.cpp
//  This file is part of the "Concurrent" project and released under the MIT License.
//
//  Created by Samuel Williams on 4/7/2017.
//  Copyright, 2017, by Samuel Williams. All rights reserved.
//

#include <UnitTest/UnitTest.hpp>

#include <Concurrent/Stack.hpp>

namespace Concurrent
{
	struct Move
	{
		std::size_t value;
		
		Move() = default;
		Move(const Move & other) = delete;
		Move(Move && other) = default;
	};
	
	UnitTest::Suite StackTestSuite {
		"Concurrent::Stack",
		
		{"should allocate a stack at least as big as requested",
			[](UnitTest::Examiner & examiner) {
				Stack stack(1024);
				
				examiner.expect(stack.size()) >= 1024;
			}
		},
		
		{"can emplace an item on the stack",
			[](UnitTest::Examiner & examiner) {
				Stack stack(1024);
				
				stack.emplace<std::size_t>(10);
				
				std::size_t * current = reinterpret_cast<std::size_t *>(stack.current());
				
				examiner.expect(*current) == 10;
			}
		},
		
		{"can push an item on the stack",
			[](UnitTest::Examiner & examiner) {
				Stack stack(1024);
				
				Move * current = stack.push(Move{10});
				
				examiner.expect(current->value) == 10;
			}
		},
	};
}
