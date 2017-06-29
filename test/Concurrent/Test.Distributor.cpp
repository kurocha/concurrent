//
//  Test.Distributor.cpp
//  This file is part of the "Concurrent" project and released under the MIT License.
//
//  Created by Samuel Williams on 29/6/2017.
//  Copyright, 2017, by Samuel Williams. All rights reserved.
//

#include <UnitTest/UnitTest.hpp>

#include <Concurrent/Distributor.hpp>
#include <functional>

namespace Concurrent
{
	UnitTest::Suite DistributorTestSuite {
		"Concurrent::Distributor",
		
		{"it can do work",
			[](UnitTest::Examiner & examiner) {
				int count = 0;
				
				{
					// Create a distributor with a queue depth of 2, with 8 threads:
					Distributor<std::function<void()>> distributor([](std::function<void()> & work){
						// Invoke the function.
						work();
					}, 2, 8);
					
					distributor([&](){
						// No guarantee of synchronization here.
						count += 1;
					});
					
					// ~Distributor waits for all jobs to complete.
				}
				
				examiner.expect(count) == 1;
			}
		},
	};
}
