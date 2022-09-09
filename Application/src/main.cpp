#include <iostream>
#include <cassert>
#include <chrono>
#include <cassert>

#include <easy/profiler.h>

#if USE_WORKING_IMPLEMENTATION
#include "workingImplementation.h"
#else
#include "exercise.h"
#endif // USE_WORKING_IMPLEMENTATION

int main()
{
	EASY_PROFILER_ENABLE;
	
	/*
	How many iterations we'll go through before returning an approximation of PI.
	Adjust this to suit your PC's performance, there's no need for it to take 5 hours to approximate PI.
	*/
	constexpr const size_t ITERATIONS = 1000000;
	
	/*
	Number of worker threads that can run an approximation subroutine at the same time.
	Set it to the number of logical cores of your CPU - 2 for consistent results:
	-1 core for the main thread and one more -1 to make it an even number for even divisions in subroutines' for loops (the ITERATIONS / NR_OF_WORKERS parts).
	*/
	constexpr const size_t NR_OF_WORKERS = 4;

	// Measure baseline algorithm.
	auto startTime = std::chrono::system_clock::now(); // Store time before running algorithm.
	const float piApprox = SingleThread(ITERATIONS);
	auto endTime = std::chrono::system_clock::now(); // Measure time having ran the algorithm.
	std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait for 100ms for ease of profiler graph reading.
	std::cout << "SingleThread has computed PI as " << std::to_string(piApprox) << " in " << std::to_string((endTime - startTime).count()) << " ticks." << std::endl;

	// Measure Async.
	startTime = std::chrono::system_clock::now();
	float pi = Async(ITERATIONS, NR_OF_WORKERS);
	endTime = std::chrono::system_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "Async has computed PI as " << std::to_string(pi) << " in " << std::to_string((endTime - startTime).count()) << " ticks." << std::endl;

	// Measure Threads.
	startTime = std::chrono::system_clock::now();
	pi = Threads(ITERATIONS, NR_OF_WORKERS);
	endTime = std::chrono::system_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "Threads has computed PI as " << std::to_string(pi) << " in " << std::to_string((endTime - startTime).count()) << " ticks." << std::endl;

	// Output easy_profiler's data.
#if BUILD_WITH_EASY_PROFILER
	const auto success = profiler::dumpBlocksToFile("profilerOutputs/session.prof");
	assert(success && "Failed to write profiling data to file.");
#endif

	return 0;
}