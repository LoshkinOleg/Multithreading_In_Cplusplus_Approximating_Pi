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

float SingleThread(const size_t iterations, const unsigned int seed)
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

float SimpleAsync(const size_t iterations, const unsigned int seed, const size_t nrOfWorkers)
{
	EASY_BLOCK("std::async method");

	std::default_random_engine e(seed);
	std::uniform_real_distribution<float> d(-1.0f, 1.0f);

	const auto approximatePi = [&e, &d](const size_t iterations, const size_t nrOfWorkers)->size_t
	{
		EASY_BLOCK("std::async routine");
		float x = 0.0f, y = 0.0f;
		size_t circle = 0;
		for (size_t i = 0; i < iterations / nrOfWorkers; i++)
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

	std::vector<std::future<size_t>> futures(nrOfWorkers);
	for (size_t worker = 0; worker < nrOfWorkers; worker++)
	{
		futures[worker] = std::async(std::launch::async, approximatePi, iterations, nrOfWorkers);
	}

	size_t insideCircle = 0;
	for (size_t worker = 0; worker < nrOfWorkers; worker++)
	{
		insideCircle += futures[worker].get();
	}

	return 4.0f * (float)insideCircle / (float)iterations;
}

float AsyncNoRef(const size_t iterations, const unsigned int seed, const size_t nrOfWorkers)
{
	EASY_BLOCK("std::async no ref method");

	const auto approximatePi = [](const size_t iterations, const size_t nrOfWorkers, const unsigned int seed)->size_t
	{
		EASY_BLOCK("std::async routine");
		std::default_random_engine e(seed);
		std::uniform_real_distribution<float> d(-1.0f, 1.0f);
		float x = 0.0f, y = 0.0f;
		size_t circle = 0;
		for (size_t i = 0; i < iterations / nrOfWorkers; i++)
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

	std::vector<std::future<size_t>> futures(nrOfWorkers);
	for (size_t worker = 0; worker < nrOfWorkers; worker++)
	{
		futures[worker] = std::async(std::launch::async, approximatePi, iterations, nrOfWorkers, seed + worker);
	}

	size_t insideCircle = 0;
	for (size_t worker = 0; worker < nrOfWorkers; worker++)
	{
		insideCircle += futures[worker].get();
	}

	return 4.0f * (float)insideCircle / (float)iterations;
}

int main()
{
	EASY_PROFILER_ENABLE;

	constexpr const size_t ITERATIONS = 10000 * 2048;
	constexpr const unsigned int SEED = 0x1337;
	constexpr const size_t NR_OF_WORKERS = 4;

	auto startTime = std::chrono::system_clock::now();
	float pi = SingleThread(ITERATIONS, SEED);
	auto endTime = std::chrono::system_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "SingleThread has computed PI as " << std::to_string(pi) << " in " << std::to_string((endTime - startTime).count()) << " ticks." << std::endl;

	startTime = std::chrono::system_clock::now();
	pi = SimpleAsync(ITERATIONS, SEED, NR_OF_WORKERS);
	endTime = std::chrono::system_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "SimpleAsync has computed PI as " << std::to_string(pi) << " in " << std::to_string((endTime - startTime).count()) << " ticks." << std::endl;

	startTime = std::chrono::system_clock::now();
	pi = AsyncNoRef(ITERATIONS, SEED, NR_OF_WORKERS);
	endTime = std::chrono::system_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "AsyncNoRef has computed PI as " << std::to_string(pi) << " in " << std::to_string((endTime - startTime).count()) << " ticks." << std::endl;

	const auto success = profiler::dumpBlocksToFile("../profilerOutputs/session.prof");
	assert(success && "Failed to write profiling data to file.");

	return 0;
}