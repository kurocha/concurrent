//
//  Distributor.hpp
//  File file is part of the "Concurrent" project and released under the MIT License.
//
//  Created by Samuel Williams on 29/6/2017.
//  Copyright, 2017, by Samuel Williams. All rights reserved.
//

#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

namespace Concurrent
{
	template <typename Type, typename Queue = std::queue<Type>>
	class Distributor : Queue, std::mutex, std::condition_variable {
		typedef typename Queue::size_type size_type;
		
		size_type _capacity;
		bool _done = false;
		std::vector<std::thread> _threads;

	public:
		template <typename Function>
		Distributor(Function function, size_type max_items_per_thread = 1, size_type concurrency = std::thread::hardware_concurrency()) : _capacity{concurrency * max_items_per_thread}
		{
			if (concurrency == 0)
				throw std::invalid_argument("Concurrency must be non-zero");
			
			for (size_type count = 0; count < concurrency; count += 1)
				_threads.emplace_back(static_cast<void (Distributor::*)(Function)>(&Distributor::consume), this, function);
		}
		
		Distributor(size_type max_items_per_thread = 1, size_type concurrency = std::thread::hardware_concurrency()) : Distributor([](Type & type){type();}, max_items_per_thread, concurrency) {}
		
		Distributor(Distributor &&) = default;
		Distributor &operator=(Distributor &&) = delete;

		~Distributor()
		{
			{
				std::lock_guard<std::mutex> guard(*this);
				_done = true;
				notify_all();
			}
			
			for (auto && thread: _threads) thread.join();
		}

		void operator()(Type &&value)
		{
			std::unique_lock<std::mutex> lock(*this);
			
			if (_capacity > 0)
				while (Queue::size() >= _capacity)
					wait(lock);
			
			Queue::push(std::forward<Type>(value));
			notify_one();
		}

	private:
		template <typename Function>
		void consume(Function process)
		{
			std::unique_lock<std::mutex> lock(*this);
			
			while (true) {
				if (!Queue::empty()) {
					Type item{std::move(Queue::front())};
					Queue::pop();
					notify_one();
					lock.unlock();
					process(item);
					lock.lock();
				} else if (_done) {
					break;
				} else {
					wait(lock);
				}
			}
		}
	};
}