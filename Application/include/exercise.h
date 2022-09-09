#include <random>
#include <thread>
#include <future>

#include <easy/profiler.h>

inline float Magnitude(const float x, const float y)
{
	return std::sqrtf(x * x + y * y);
}

// Approximates PI on a single thread. Baseline case to compare against.
float SingleThread(const size_t iterations)
{
	EASY_BLOCK("SingleThread approach.", profiler::colors::Green);

	std::default_random_engine e; // Random engine we'll be using to generate random floats.
	std::uniform_real_distribution<float> d(-1.0f, 1.0f); // We're going to be generating uniformly distrubuted floats (meaning no particular pattern, not even normally distrubuted).

	size_t insideCircle = 0;

	/*TODO:
		Approximate PI using the Monte Carlo approach: https://www.geeksforgeeks.org/estimating-value-pi-using-monte-carlo/
	*/

	return 0.0f; // Compute approximation of PI using the ratio of points inside the unit circle vs. points inside the unit square.
}

// Approximates PI by kicking off smaller pi approximating subroutines but lets them instanciate their own random number generators.
float Async(const size_t iterations, const size_t nrOfWorkers)
{
	EASY_BLOCK("Async method.", profiler::colors::Red);

	// Implementation of the PI approximating algorithm but this time split into multiple subroutines with their own random number generators.
	const auto approximatePi = [](const size_t iterations, const size_t nrOfWorkers, const size_t workerId)->size_t
	{
		EASY_BLOCK("Approximation subroutine.", profiler::colors::Red100);

		/*TODO:
			Generate 2D points within a unit square using e, d, iterationsand nrOfWorkersand count the number of points that lies inside the unit circle.
			The idea is that each subroutine should only do the iterations / nrOfWorkers amount of iterations and you'll then sum up the results of all the
			subroutines to obtain the equivalent approximation of the SingleThread approach.
			Use local random number generators.
		*/

		return 0;
	};

	std::vector<std::future<size_t>> futures(nrOfWorkers);
	{
		EASY_BLOCK("Kicking off threads.", profiler::colors::Red100);
		
		/*TODO:
			Kick off std::async's executing approximatePi.
		*/
	}

	size_t insideCircle = 0;
	{
		EASY_BLOCK("Retrieving results.", profiler::colors::Red100);
		
		/*TODO:
			Retrieve the results of the subroutines computed asynchnously.
		*/
	}

	return 0.0f;
}

// Approximates PI by kicking off smaller pi approximating subroutines guaranteed to be on different threads and lets them instanciate their own random number generators.
float Threads(const size_t iterations, const size_t nrOfWorkers)
{
	EASY_BLOCK("Threads method.", profiler::colors::Blue);

	// Modified version of approximatePi that uses a std::promise to return the result instead of the return value of the function.
	const auto approximatePi = [](/* TODO: Modify the signature of this lambda so that it uses a std::promise to return a value instead of the regular return value. */)
	{
		EASY_BLOCK("Approximation subroutine.", profiler::colors::Blue100);
		
		/*TODO:
			Implement an approximatePi like the one from AsyncNoRef but this time use a std::promise to return the result.
		*/
	};

	std::vector<std::thread> threads; // Vector for all the threads we'll be kicking off.
	std::vector<std::future<size_t>> futures; // And a vector for holding their associated futures to retireve their results.
	{
		EASY_BLOCK("Kicking off threads.", profiler::colors::Blue100);
		
		/*TODO:
			Kick off std::async's executing approximatePi.
		*/
	}

	size_t insideCircle = 0;
	{
		EASY_BLOCK("Retrieving results.", profiler::colors::Blue100);
		
		/*TODO:
			Retrieve the results of the subroutines computed asynchnously.
		*/
	}

	return 0.0f;
}