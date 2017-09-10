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
					Distributor<std::function<void()>> distributor;
					
					// This distributor can handle 16 items in it's work queue. After that, it will block the calling thread.
					// Alternatively, specify 0 for the queue depth and it will accept an unlimited number of jobs.
					distributor([&]{
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
