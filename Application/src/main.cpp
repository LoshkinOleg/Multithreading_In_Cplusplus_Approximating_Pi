// Taken from: https://www.geeksforgeeks.org/estimating-value-pi-using-monte-carlo/

#include <random>
#include <cmath>
#include <array>
#include <iostream>
#include <string>
#include <cassert>
#include <future>
#include <chrono>
#include <thread>

#include <easy/profiler.h>

inline float Magnitude(const float x, const float y)
{
	return std::sqrtf(x * x + y * y);
}

// Baseline to compare against for our operation: fully sequential with the compiler left to optimize a for loop as it wants.
float ApproximateSingleThread(const size_t iterations, const unsigned int seed)
{
	EASY_BLOCK("Single thread");

	std::default_random_engine e(seed);
	std::uniform_real_distribution<float> d(-1.0f, 1.0f);

	float x = 0.0f, y = 0.0f;
	size_t circle = 0;
	for (size_t i = 0; i < iterations; i++)
	{
		x = d(e);
		y = d(e);
		if (Magnitude(x, y) <= 1.0f)
		{
			circle++;
		}
	}

	return 4.0f * (float)circle / (float)iterations;
}

// Std::async based methods: these don't do what we want since we have no control over parallelization parameters with it.
// It is a good way to quickly throw some task onto another thread for execution though. It's like C#'s coroutines.
//==========================
// Yields the same performance as ApproximateSingleThread but chopped up in 8 on a single thread. Not what we want.
float ApproximateWithAsyncNoWait(const size_t iterations, const unsigned int seed)
{
	constexpr const size_t NR_OF_WORKERS = 7;

	EASY_BLOCK("std::async method");

	std::default_random_engine e(seed);
	std::uniform_real_distribution<float> d(-1.0f, 1.0f);

	const auto approximatePi = [&e, &d](const size_t iterations)->size_t
	{
		EASY_BLOCK("std::async routine");
		float x = 0.0f, y = 0.0f;
		size_t circle = 0;
		for (size_t i = 0; i < iterations / NR_OF_WORKERS; i++)
		{
			x = d(e);
			y = d(e);
			if (Magnitude(x, y) <= 1.0f)
			{
				circle++;
			}
		}
		EASY_END_BLOCK;
		return circle;
	};

	size_t insideCircle = 0;
	for (size_t worker = 0; worker < NR_OF_WORKERS; worker++)
	{
		//	This results in a sequential operation on a separate thread. \
			Would be also true if we didn't .get() the result: the destructor of the std::future waits \
			for the end of the execution, which is why it's important to store the future in a variable \
			to allow the local thread to keep executing.
		insideCircle += std::async(std::launch::async, approximatePi, iterations).get();
	}

	return 4.0f * (float)insideCircle / (float)iterations;
}

// Yields the same performance as ApproximateSingleThread but with all 8 threads working instead of one, essentially doing 8 times the work in the same amount of time for the same result. Still not what we want.
float ApproximateWithAsync(const size_t iterations, const unsigned int seed)
{
	constexpr const size_t NR_OF_WORKERS = 7;

	EASY_BLOCK("std::async method");

	std::default_random_engine e(seed);
	std::uniform_real_distribution<float> d(-1.0f, 1.0f);

	const auto approximatePi = [&e, &d](const size_t iterations)->size_t
	{
		EASY_BLOCK("std::async routine");
		float x = 0.0f, y = 0.0f;
		size_t circle = 0;
		for (size_t i = 0; i < iterations / NR_OF_WORKERS; i++)
		{
			x = d(e);
			y = d(e);
			if (Magnitude(x, y) <= 1.0f)
			{
				circle++;
			}
		}
		EASY_END_BLOCK;
		return circle;
	};

	std::array<std::future<size_t>, NR_OF_WORKERS> futures;
	for (size_t worker = 0; worker < NR_OF_WORKERS; worker++)
	{
		futures[worker] = std::async(std::launch::async, approximatePi, iterations);
	}

	size_t insideCircle = 0;
	for (size_t worker = 0; worker < NR_OF_WORKERS; worker++)
	{
		insideCircle += futures[worker].get();
	}

	return 4.0f * (float)insideCircle / (float)iterations;
}

// Yields a similar result to that of ApproximateWithAsyncNoWait(): sequential calls to approximatePi but this time on the same thread. Not what we want either.
float ApproximateWithDeferred(const size_t iterations, const unsigned int seed)
{
	constexpr const size_t NR_OF_WORKERS = 7;

	EASY_BLOCK("std::async method");

	std::default_random_engine e(seed);
	std::uniform_real_distribution<float> d(-1.0f, 1.0f);

	const auto approximatePi = [&e, &d](const size_t iterations)->size_t
	{
		EASY_BLOCK("std::async routine");
		float x = 0.0f, y = 0.0f;
		size_t circle = 0;
		for (size_t i = 0; i < iterations / NR_OF_WORKERS; i++)
		{
			x = d(e);
			y = d(e);
			if (Magnitude(x, y) <= 1.0f)
			{
				circle++;
			}
		}
		EASY_END_BLOCK;
		return circle;
	};

	std::array<std::future<size_t>, NR_OF_WORKERS> futures;
	for (size_t worker = 0; worker < NR_OF_WORKERS; worker++)
	{
		futures[worker] = std::async(std::launch::deferred, approximatePi, iterations);
	}

	size_t insideCircle = 0;
	for (size_t worker = 0; worker < NR_OF_WORKERS; worker++)
	{
		insideCircle += futures[worker].get();
	}

	return 4.0f * (float)insideCircle / (float)iterations;
}
//==========================

int main()
{
	EASY_PROFILER_ENABLE;

	constexpr const size_t ITERATIONS = 10000 * 2048;
	constexpr const unsigned int SEED = 0x1337;

	auto startTime = std::chrono::system_clock::now();
	float pi = ApproximateSingleThread(ITERATIONS, SEED);
	auto endTime = std::chrono::system_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "ApproximateSingleThread has computed PI as " << std::to_string(pi) << " in " << std::to_string((endTime - startTime).count()) << " ticks." << std::endl;

	startTime = std::chrono::system_clock::now();
	pi = ApproximateWithAsyncNoWait(ITERATIONS, SEED);
	endTime = std::chrono::system_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "ApproximateWithAsyncNoWait has computed PI as " << std::to_string(pi) << " in " << std::to_string((endTime - startTime).count()) << " ticks." << std::endl;

	startTime = std::chrono::system_clock::now();
	pi = ApproximateWithAsync(ITERATIONS, SEED);
	endTime = std::chrono::system_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "ApproximateWithAsync has computed PI as " << std::to_string(pi) << " in " << std::to_string((endTime - startTime).count()) << " ticks." << std::endl;

	startTime = std::chrono::system_clock::now();
	pi = ApproximateWithDeferred(ITERATIONS, SEED);
	endTime = std::chrono::system_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "ApproximateWithDeferred has computed PI as " << std::to_string(pi) << " in " << std::to_string((endTime - startTime).count()) << " ticks." << std::endl;

	const auto success = profiler::dumpBlocksToFile("../profilerOutputs/session.prof");
	assert(success && "Failed to write profiling data to file.");

	return 0;
}