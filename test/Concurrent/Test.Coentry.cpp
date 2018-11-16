//
//  Test.Coentry.cpp
//  This file is part of the "Concurrent" project and released under the MIT License.
//
//  Created by Samuel Williams on 4/7/2017.
//  Copyright, 2017, by Samuel Williams. All rights reserved.
//

#include <UnitTest/UnitTest.hpp>

#include <Concurrent/Coentry.hpp>
#include <Concurrent/Stack.hpp>

namespace Concurrent
{
	struct Counter
	{
		std::size_t * copies, * moves;
		
		Counter(std::size_t * copies_, std::size_t * moves_) : copies(copies_), moves(moves_) {}
		Counter(const Counter & other) : copies(other.copies), moves(other.moves) {*copies += 1;}
		Counter & operator=(const Counter & other) {copies = other.copies; moves = other.moves; *copies += 1; return *this;}
		Counter(Counter && other) : copies(other.copies), moves(other.moves) {*moves += 1;}
		Counter & operator=(Counter && other) {copies = other.copies; moves = other.moves; *moves += 1; return *this;}
	};
	
	UnitTest::Suite CoentryTestSuite {
		"Concurrent::Coentry",
		
		{"it can invoke lambda function on a stack",
			[](UnitTest::Examiner & examiner) {
				std::size_t copies = 0, moves = 0;
				
				Stack stack(1024);
				int x = 0;
				
				auto coentry = emplace_coentry(stack, [&, c = Counter(&copies, &moves)]{
					x += 1;
				});
				
				coentry->function();
				
				examiner.expect(x) == 1;
				examiner.expect(copies) == 0;
				examiner.expect(moves) == 1;
			}
		},
	};
}
