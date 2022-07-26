# Concurrent

Provides basic concurrency primitives, including stackful coroutines.

## Motivation

Coroutines are [a negative overhead abstraction](https://www.youtube.com/watch?v=_fu0gx-xseY). They allow for efficient, expressive code which is superior to manual state tracking. Normal flow control is not disturbed by structures required for concurrent execution.

This minimises the cognitive overhead of dealing with both execution logic and asynchronous logic at the same time. Compare the following:

```c++
// According to N4399 Working Draft
future<void> do_while(std::function<future<bool>()> body) {
	return body().then([=](future<bool> notDone) {
		return notDone.get() ? do_while(body) : make_ready_future();
	});
}

future<int> tcp_reader(int64_t total) {
	struct State {
		char buf[4 * 1024];
		int64_t total;
		Tcp::Connection conn;
		explicit State(int64_t total) : total(total) {}
	};

	auto state = make_shared<State>(total);
	
	return Tcp::Connect("127.0.0.1", 1337).then(
		[state](future<Tcp::Connection> conn) {
			state->conn = std::move(conn.get());
			return do_while([state]()->future<bool> {
				if (state->total <= 0)
					return make_ready_future(false);
				
				return state->conn.read(state->buf, sizeof(state->buf)).then(
					[state](future<int> nBytesFut) {
						auto nBytes = nBytesFut.get();
						
						if (nBytes == 0)
							return make_ready_future(false);
						
						state->total -= nBytes;
						return make_ready_future(true);
					}
				);
			});
		}
	).then([state](future<void>) {
		return make_ready_future(state->total)
	}); 
}
```

with:

```c++
int tcp_reader(int total)
{
	char buf[4 * 1024];
	auto conn = Tcp::Connect("127.0.0.1", 1337);
	for (;;) {
		auto bytesRead = conn.Read(buf, sizeof(buf));
		total -= bytesRead;
		
		if (total <= 0 || bytesRead == 0)
			return total;
	}
}
```

Not only is this simpler, it's also faster (better throughput). You can implement code like this using an [event-driven reactor](https://github.com/kurocha/async).

### Useful Definitions

- **Parallel** programs distribute their tasks to multiple processors, that actively work on all of them simultaneously.
- **Concurrent** programs handle tasks that are all in progress at the same time, but it is only necessary to work briefly and separately on each task, so the work can be interleaved in whatever order the tasks require.
- **Asynchronous**: programs dispatch tasks to devices that can take care of themselves, leaving the program free do something else until it receives a signal that the results are available.

Thanks to Jan Christian Meyer, Ph.D. in Computer Science, for these concise definitions.

## Setup

Firstly the build tool `teapot` needs to be installed (which requires [Ruby][2]):

	$ gem install teapot

To fetch all dependencies, run:

	$ teapot fetch

[2]: http://www.ruby-lang.org/en/downloads/

## Usage

To run unit tests:

	$ teapot Test/Concurrent

### Fibers

`Concurrent::Fiber` provides cooperative multi-tasking.

```c++
int x = 10;

Fiber fiber([&]{
	x = 20;
	Fiber::current->yield();
	x = 30;
});

fiber.resume();
// x is now 20.
fiber.resume();
// x is now 30.
```

The implementation uses `Concurrent::Stack` to allocate (`mmap`) a stack, in which the given lambda is allocated using `Concurrent::Coentry`.

The stack includes guard pages to protect against stack overflow.

There is a `Concurrent::Condition` primitive which allows synchronisation between fibers.

#### Fiber Pool

If you have a server which is allocating a fiber per request, use a `Concurrent::Fiber::Pool`. This reuses stacks to minimse per-request overhead.

```c++
Concurrent::Fiber::Pool pool;

// Server accept loop:
while (...) {
	pool.resume([&]{
		// Per-request work...
	});
}
```

### Distributor

`Concurrent::Distributor` provides a multi-threaded work queue.

```c++
Distributor<std::function<void()>> distributor;

distributor([&]{
	do_work();
});
```

A distributor schedules work over available hardware processors. It is useful for implementing a job queue.

## Contributing

1. Fork it.
2. Create your feature branch (`git checkout -b my-new-feature`).
3. Commit your changes (`git commit -am 'Add some feature'`).
4. Push to the branch (`git push origin my-new-feature`).
5. Create new Pull Request.

## License

Released under the MIT license.

Copyright, 2017, by [Samuel G. D. Williams](http://www.codeotaku.com/samuel-williams).

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
