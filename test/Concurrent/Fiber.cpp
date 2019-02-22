//
//  Test.Fiber.cpp
//  This file is part of the "Memory" project and released under the MIT License.
//
//  Created by Samuel Williams on 28/6/2017.
//  Copyright, 2017, by Samuel Williams. All rights reserved.
//

#include <UnitTest/UnitTest.hpp>

#include "Concurrent/Fiber.hpp"

namespace Concurrent
{
	using namespace UnitTest::Expectations;
	
	static std::ostream & operator<<(std::ostream & output, const Status & status)
	{
		if (status == Status::MAIN) {
			return output << "MAIN";
		} else if (status == Status::READY) {
			return output << "READY";
		} else if (status == Status::RUNNING) {
			return output << "RUNNING";
		} else if (status == Status::STOPPED) {
			return output << "STOPPED";
		} else if (status == Status::FINISHED) {
			return output << "FINISHED";
		} else {
			return output << "???";
		}
	}
	
	UnitTest::Suite FiberTestSuite {
		"Concurrent::Fiber",
		
		{"it should resume",
			[](UnitTest::Examiner & examiner) {
				int x = 10;
				
				Fiber fiber([&]{
					x = 20;
				});
				
				fiber.resume();
				
				examiner.expect(x) == 20;
			}
		},
		
		{"it should yield",
			[](UnitTest::Examiner & examiner) {
				int x = 10;
				
				Fiber fiber([&]{
					x = 20;
					Fiber::current->yield();
					x = 30;
				});
				
				fiber.resume();
				examiner.expect(x) == 20;
				
				fiber.resume();
				examiner.expect(x) == 30;
			}
		},
		
		{"it should throw exceptions",
			[](UnitTest::Examiner & examiner) {
				Fiber fiber([&]{
					throw std::logic_error("your logic has failed me");
				});
				
				examiner.expect([&]{
					fiber.resume();
				}).to(throw_exception<std::logic_error>());
			}
		},
		
		{"it can be stopped",
			[](UnitTest::Examiner & examiner) {
				int count = 0;
				
				Fiber fiber([&]{
					while (true) {
						count += 1;
						Fiber::current->yield();
					}
				});
				
				examiner.expect(fiber.status()) == Status::READY;
				
				fiber.resume();
				
				examiner.expect(count) == 1;
				examiner.expect(fiber.status()) == Status::RUNNING;
				
				fiber.stop();
				
				examiner.expect(fiber.status()) == Status::FINISHED;
			}
		},
		
		{"it should resume in a nested fiber",
			[](UnitTest::Examiner & examiner) {
				std::string order;
				
				order += 'A';
				
				Fiber outer([&]{
					order += 'B';
					
					Fiber inner([&]{
						order += 'C';
					});
					
					order += 'D';
					inner.resume();
					order += 'E';
				});
				
				order += 'F';
				outer.resume();
				order += 'G';
				
				examiner.expect(order) == "AFBDCEG";
			}
		},
		
		{"it can allocate fibers from a pool",
			[](UnitTest::Examiner & examiner) {
				std::string order;
				
				// A pool groups together fibers and will stop them once it goes out of scope.
				Fiber::Pool pool;
				
				std::size_t count = 0;
				for (std::size_t i = 0; i < 5; i += 1) {
					pool.resume([&]{
						count += 1;
					});
				}
				
				examiner.expect(count) == 5;
			}
		},
	};
}
