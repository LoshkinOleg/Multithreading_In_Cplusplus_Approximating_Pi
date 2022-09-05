// Taken from: https://www.geeksforgeeks.org/estimating-value-pi-using-monte-carlo/

#include <random>
#include <cmath>
#include <array>
#include <iostream>
#include <string>

struct Point
{
	float x = 0.0f, y = 0.0f;

	inline float Magnitude() const
	{
		return std::sqrtf(x*x + y*y);
	}
};

int main()
{
	size_t circle = 0, square = 0;

	std::default_random_engine e;
	std::uniform_real_distribution<float> d(-1.0f, 1.0f);

	Point p;
	for (size_t i = 0; i < 1000 * 2048; i++)
	{
		p.x = d(e);
		p.y = d(e);
		if (p.Magnitude() <= 1.0f)
		{
			circle++;
		}
		square++;
	}

	const float pi = 4.0f * (float)circle / (float)square;
	std::cout << "PI approximation is: " << std::to_string(pi) << std::endl;
	return 0;
}