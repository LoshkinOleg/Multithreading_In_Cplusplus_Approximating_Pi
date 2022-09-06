#include <random>
#include <iostream>
#include <cassert>
#include <thread>
#include <future>

#include <easy/profiler.h>

// TODO: before posting, look into why approximations differ despite the same seed being used!

inline float Magnitude(const float x, const float y)
{
	return std::sqrtf(x * x + y * y);
}

// Approximates PI on a single thread. Baseline case to compare against.
float SingleThread(const size_t iterations, const unsigned int seed)
{
	EASY_BLOCK("SingleThread approach.", profiler::colors::Green);

	std::default_random_engine e(seed); // Random engine we'll be using to generate random floats.
	std::uniform_real_distribution<float> d(-1.0f, 1.0f); // We're going to be generating uniformly distrubuted floats (meaning no particular pattern, not even normally distrubuted).

	size_t insideCircleCount = 0;
	float x = 0.0f, y = 0.0f;
	for (size_t i = 0; i < iterations; i++)
	{
		x = d(e); // Generate random float.
		y = d(e);
		if (Magnitude(x, y) <= 1.0f) // If point lies inside the circle, increment.
		{
			insideCircleCount++;
		}
	}

	return 4.0f * (float)insideCircleCount / (float)iterations; // Compute approximation of PI using the ratio of points inside the unit circle vs. points inside the unit square.
}

// Approximates PI by kicking off smaller pi approximating subroutines and passes the random number generator by reference to them.
float SimpleAsync(const size_t iterations, const unsigned int seed, const size_t nrOfWorkers)
{
	EASY_BLOCK("SimpleAsync approach.", profiler::colors::Red);

	std::default_random_engine e(seed);
	std::uniform_real_distribution<float> d(-1.0f, 1.0f);

	// Subroutine lambda we'll be trying to launch on a separate threads. Note that the random engine and distribution is captured by reference.
	const auto approximatePi = [&e, &d](const size_t iterations, const size_t nrOfWorkers)->size_t
	{
		EASY_BLOCK("Approximation subroutine.", profiler::colors::Red100);
		float x = 0.0f, y = 0.0f;
		size_t insideCircleCount = 0;
		for (size_t i = 0; i < iterations / nrOfWorkers; i++)
		{
			x = d(e);
			y = d(e);
			if (Magnitude(x, y) <= 1.0f)
			{
				insideCircleCount++;
			}
		}
		return insideCircleCount;
	};

	std::vector<std::future<size_t>> futures; // std::futures we'll be using to retireving the results of approximatePi's once they've done running.
	{
		EASY_BLOCK("Kicking off threads.", profiler::colors::Red);
		for (size_t worker = 0; worker < nrOfWorkers; worker++)
		{
			// (Hopefully) kicking off new threads to approximate pi in a parallel manner on multiple threads. std::async doesn't guarantee that a new thread will be started!
			futures.push_back(std::async(std::launch::async, approximatePi, iterations, nrOfWorkers)); // Note that we're passing std::launch::async to indicate that we want our coroutines to (hopefully) run on a separate thread. No guarantees with std::async though.
		}
	}

	size_t insideCircle = 0;
	{
		// Retireve the results from the (hopefully) other threads once it's done computing things.
		EASY_BLOCK("Retrieving results.", profiler::colors::Red);
		for (size_t worker = 0; worker < nrOfWorkers; worker++)
		{
			EASY_BLOCK("Getting result of a single thread.", profiler::colors::Red);
			insideCircle += futures[worker].get(); // Blocks the main thread until a valid result is retrieved.
		}
	}

	return 4.0f * (float)insideCircle / (float)iterations;
}

// Approximates PI by kicking off smaller pi approximating subroutines but lets them instanciate their own random number generators.
float AsyncNoRef(const size_t iterations, const unsigned int seed, const size_t nrOfWorkers)
{
	EASY_BLOCK("AsyncNoRef method.", profiler::colors::Blue);

	// New implementation of approximatePi that constructs it's own random engine rather than capturing one by reference.
	const auto approximatePi = [](const size_t iterations, const size_t nrOfWorkers, const unsigned int seed)->size_t
	{
		EASY_BLOCK("Approximation subroutine.", profiler::colors::Blue100);
		std::default_random_engine e(seed);
		std::uniform_real_distribution<float> d(-1.0f, 1.0f);
		float x = 0.0f, y = 0.0f;
		size_t insideCircle = 0;
		for (size_t i = 0; i < iterations / nrOfWorkers; i++)
		{
			x = d(e);
			y = d(e);
			if (Magnitude(x, y) <= 1.0f)
			{
				insideCircle++;
			}
		}
		return insideCircle;
	};

	std::vector<std::future<size_t>> futures(nrOfWorkers);
	{
		EASY_BLOCK("Kicking off threads.", profiler::colors::Blue);
		for (size_t worker = 0; worker < nrOfWorkers; worker++)
		{
			futures[worker] = std::async(std::launch::async, approximatePi, iterations, nrOfWorkers, seed + worker); // Note that we're passing seed + worker to ensure that all the random engines generate different numbers.
		}
	}

	size_t insideCircle = 0;
	{
		EASY_BLOCK("Retrieving results.", profiler::colors::Blue);
		for (size_t worker = 0; worker < nrOfWorkers; worker++)
		{
			EASY_BLOCK("Getting result of a single thread.", profiler::colors::Blue);
			insideCircle += futures[worker].get(); // Blocks the main thread until a valid result is retrieved.
		}
	}

	return 4.0f * (float)insideCircle / (float)iterations;
}

// Approximates PI by kicking off smaller pi approximating subroutines guaranteed to be on different threads and lets them instanciate their own random number generators.
float Threads(const size_t iterations, const unsigned int seed, const size_t nrOfWorkers)
{
	EASY_BLOCK("Threads method.", profiler::colors::Yellow);

	// Modified version of approximatePi that uses a std::promise to return the result instead of the return value of the function.
	const auto approximatePi = [](std::promise<size_t>&& returnVal, const size_t iterations, const size_t nrOfWorkers, const unsigned int seed)
	{
		EASY_BLOCK("Approximation subroutine.", profiler::colors::Yellow100);
		std::default_random_engine e(seed);
		std::uniform_real_distribution<float> d(-1.0f, 1.0f);
		float x = 0.0f, y = 0.0f;
		size_t insideCircle = 0;
		for (size_t i = 0; i < iterations / nrOfWorkers; i++)
		{
			x = d(e);
			y = d(e);
			if (Magnitude(x, y) <= 1.0f)
			{
				insideCircle++;
			}
		}
		returnVal.set_value(insideCircle);
	};

	std::vector<std::thread> threads; // Vector for all the threads we'll be kicking off.
	std::vector<std::future<size_t>> futures; // And a vector for holding their associated futures to retireve their results.
	{
		EASY_BLOCK("Kicking off threads.", profiler::colors::Yellow);
		for (size_t worker = 0; worker < nrOfWorkers; worker++)
		{
			std::promise<size_t> p; // Construct a promise to pass to the subroutine it'll use to return the result.
			futures.push_back(p.get_future());
			threads.push_back(std::thread(approximatePi, std::move(p), iterations, nrOfWorkers, seed + (unsigned int)worker)); // Note that we're std::move'ing the std::promise.
		}
	}

	size_t insideCircle = 0;
	{
		EASY_BLOCK("Retrieving results.", profiler::colors::Yellow);
		for (size_t worker = 0; worker < nrOfWorkers; worker++)
		{
			EASY_BLOCK("Getting result of a single thread.", profiler::colors::Yellow);
			// threads[worker].join(); // Blocks the main thread until a valid result is retrieved. Failing to do this results in an exception.
			threads[worker].detach(); // Alternatively, we could just detach the thread and let it run. The std::future.get() won't let us continue unless the future is valid anyways, meaning the thread is done.
			insideCircle += futures[worker].get();
		}
	}

	return 4.0f * (float)insideCircle / (float)iterations;
}

int main()
{
	EASY_PROFILER_ENABLE;

	// Seed for the random number generator to ensure consistency between runs.
	constexpr const unsigned int SEED = 0x1337;
	
	// How many iterations we'll go through before returning an approximation of PI.
	constexpr const size_t ITERATIONS = 10000000;
	
	//	Number of worker threads that can run an approximation subroutine at the same time. \
		Set it to the number of logical cores of your CPU - 2 for consistent results: \
		-1 core for the main thread and one more -1 to make it an even number for even divisions in subroutines' for loops (the ITERATIONS / NR_OF_WORKERS part).
	constexpr const size_t NR_OF_WORKERS = 4;

	// Measure baseline algorithm.
	auto startTime = std::chrono::system_clock::now(); // Store time before running algorithm.
	float pi = SingleThread(ITERATIONS, SEED);
	auto endTime = std::chrono::system_clock::now(); // Measure time having ran the algorithm.
	std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait for 100ms for ease of profiler graph reading.
	std::cout << "SingleThread has computed PI as " << std::to_string(pi) << " in " << std::to_string((endTime - startTime).count()) << " ticks." << std::endl;

	// Measure SimpleAsync.
	startTime = std::chrono::system_clock::now();
	pi = SimpleAsync(ITERATIONS, SEED, NR_OF_WORKERS);
	endTime = std::chrono::system_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "SimpleAsync has computed PI as " << std::to_string(pi) << " in " << std::to_string((endTime - startTime).count()) << " ticks." << std::endl;

	// Measure AsyncNoRef.
	startTime = std::chrono::system_clock::now();
	pi = AsyncNoRef(ITERATIONS, SEED, NR_OF_WORKERS);
	endTime = std::chrono::system_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "AsyncNoRef has computed PI as " << std::to_string(pi) << " in " << std::to_string((endTime - startTime).count()) << " ticks." << std::endl;

	// Measure Threads.
	startTime = std::chrono::system_clock::now();
	pi = Threads(ITERATIONS, SEED, NR_OF_WORKERS);
	endTime = std::chrono::system_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "Threads has computed PI as " << std::to_string(pi) << " in " << std::to_string((endTime - startTime).count()) << " ticks." << std::endl;

	// Output easy_profiler's data.
	const auto success = profiler::dumpBlocksToFile("../profilerOutputs/session.prof");
	assert(success && "Failed to write profiling data to file.");

	return 0;
}