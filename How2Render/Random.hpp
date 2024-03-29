#pragma once

#include "Math.hpp"
#include <chrono>
#include <cstdint>
#include <random>

class splitmix
{
public:
	using result_type = uint32_t;
	static constexpr result_type(min)() { return 0; }
	static constexpr result_type(max)() { return UINT32_MAX; }

	splitmix() : m_seed(1) {}
	explicit splitmix(uint64_t seed) : m_seed(seed << 31 | seed) {}
	explicit splitmix(std::random_device &rd)
	{
		seed(rd);
	}

	void seed(std::random_device &rd)
	{
		m_seed = uint64_t(rd()) << 31 | uint64_t(rd());
	}

	result_type operator()()
	{
		uint64_t z = (m_seed += UINT64_C(0x9E3779B97F4A7C15));
		z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
		z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
		return result_type((z ^ (z >> 31)) >> 31);
	}

	void discard(unsigned long long n)
	{
		for (unsigned long long i = 0; i < n; ++i)
		{
			operator()();
		}
	}

private:
	uint64_t m_seed;
};

inline __forceinline float GenerateUniformRealDist(float min, float max)
{
	static splitmix rand(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()));
	std::uniform_real_distribution<float> u(min, max);

	return u(rand);
}

inline __forceinline XMVECTOR GenerateNormalDistInsideSphereVector(float radius)
{
	XMVECTOR result;

	do
	{
		result = {GenerateUniformRealDist(-radius, radius),
				  GenerateUniformRealDist(-radius, radius),
				  GenerateUniformRealDist(-radius, radius)};
	} while (XMVector3LengthSq(result).m128_f32[0] > radius * radius);

	return result;
}
