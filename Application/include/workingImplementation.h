#include <random>
#include <thread>
#include <future>

#include <easy/profiler.h>

/*
	Disclaimer: this implementation does not ensure that the approximations generated are identical!
	This example only demonstrates how to use std::async and std::threads.
	Making sure the approximations are identical is the subject of the next blogpost which will talk about sharing of resources and data races.
*/

inline float Magnitude(const float x, const float y)
{
	return std::sqrtf(x * x + y * y);
}

// Approximates PI on a single thread. Baseline case to compare against.
float SingleThread(const size_t iterations)
{
	EASY_BLOCK("SingleThread approach.", profiler::colors::Green);

	// Default seed is: 5489 (unsigned).
	std::default_random_engine e; // Random engine we'll be using to generate random floats.
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

// Approximates PI by kicking off smaller pi approximating subroutines but lets them instanciate their own random number generators.
float Async(const size_t iterations, const size_t nrOfWorkers)
{
	EASY_BLOCK("Async method.", profiler::colors::Red);

	// Implementation of the PI approximating function, but this time with a local random engine.
	const auto approximatePi = [](const size_t iterations, const size_t nrOfWorkers, const size_t workerId)->size_t
	{
		EASY_BLOCK("Approximation subroutine.", profiler::colors::Red100);
		std::default_random_engine e(workerId);
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
		EASY_BLOCK("Kicking off threads.", profiler::colors::Red100);
		for (size_t worker = 0; worker < nrOfWorkers; worker++)
		{
			futures[worker] = std::async(std::launch::async, approximatePi, iterations, nrOfWorkers, worker); // Note that we're passing seed + worker to ensure that all the random engines generate different numbers.
		}
	}

	size_t insideCircle = 0;
	{
		EASY_BLOCK("Retrieving results.", profiler::colors::Red100);
		for (size_t worker = 0; worker < nrOfWorkers; worker++)
		{
			EASY_BLOCK("Getting result of a single thread.", profiler::colors::Red100);
			insideCircle += futures[worker].get(); // Blocks the main thread until a valid result is retrieved.
		}
	}

	return 4.0f * (float)insideCircle / (float)iterations;
}

// Approximates PI by kicking off smaller pi approximating subroutines guaranteed to be on different threads and lets them instanciate their own random number generators.
float Threads(const size_t iterations, const size_t nrOfWorkers)
{
	EASY_BLOCK("Threads method.", profiler::colors::Blue);

	// Modified version of approximatePi that uses a std::promise to return the result instead of the return value of the function.
	const auto approximatePi = [](std::promise<size_t>&& returnVal, const size_t iterations, const size_t nrOfWorkers, const size_t workerId)
	{
		EASY_BLOCK("Approximation subroutine.", profiler::colors::Blue100);
		std::default_random_engine e(workerId);
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
		EASY_BLOCK("Kicking off threads.", profiler::colors::Blue100);
		for (size_t worker = 0; worker < nrOfWorkers; worker++)
		{
			std::promise<size_t> p; // Construct a promise to pass to the subroutine it'll use to return the result.
			futures.push_back(p.get_future());
			threads.push_back(std::thread(approximatePi, std::move(p), iterations, nrOfWorkers, worker)); // Note that we're std::move'ing the std::promise.
		}
	}

	size_t insideCircle = 0;
	{
		EASY_BLOCK("Retrieving results.", profiler::colors::Blue100);
		for (size_t worker = 0; worker < nrOfWorkers; worker++)
		{
			EASY_BLOCK("Getting result of a single thread.", profiler::colors::Blue100);
			threads[worker].join(); // Blocks the main thread until a valid result is retrieved. Failing to do this results in an exception.
			// threads[worker].detach(); // Alternatively, we could just detach the thread and let it run. The std::future.get() won't let us continue unless the future is valid anyways, meaning the thread is done.
			insideCircle += futures[worker].get();
		}
	}

	return 4.0f * (float)insideCircle / (float)iterations;
}