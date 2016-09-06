#include "Misc.h"
#ifdef _WIN32
#include <Windows.h>
#endif
#include <cstring>
#include <random>

void *AlignedMalloc(size_t size, size_t alignment)
{
	void *ptr = nullptr;
#ifdef _WIN32
	ptr = _aligned_malloc(size, alignment);
	// TODO: Other platforms
#endif
	return ptr;
}

void AlignedFree(void *ptr) {
#ifdef _WIN32
	_aligned_free(ptr);
#endif
}


std::mt19937 BonoboRandom(1 | (0xBABEFACE ^ rand()));

#ifdef max
#undef max
#endif

void RandomSeed(unsigned int seed)
{
	BonoboRandom = std::mt19937(seed);
}

double RandomUniform()
{
	return double(BonoboRandom()) / (double(BonoboRandom.max()) + 1.0);
}

double RandomUniform(double from, double to)
{
	return from + (to - from) * RandomUniform();
}

void *InfuseData(void *arrayA, size_t strideA, size_t offsetInA,
				 void *arrayB, size_t strideB, size_t offsetInB, size_t sizeB, size_t n)
{
	size_t newStride = strideA + sizeB;
	unsigned char *newArray = (unsigned char *) malloc(n * newStride);
	unsigned char *a_pre = ((unsigned char *) arrayA);
	unsigned char *a_post = ((unsigned char *) arrayA) + offsetInA;
	size_t size_post = (strideA - offsetInA);
	unsigned char *b = ((unsigned char *) arrayB) + offsetInB;
	unsigned char *d = newArray;

	// LOWPRIO: Can be done with 2 memcpy's (+ pre and post)
	for (size_t i = 0; i < n; i++) {
		memcpy(d, a_pre, offsetInA);
		a_pre += strideA;
		d += offsetInA;
		memcpy(d, b, sizeB);
		b += strideB;
		d += sizeB;
		memcpy(d, a_post, size_post);
		a_post += strideA;
		d += size_post;
	}

	return newArray;
}

double GetTimeSeconds()
{
	return static_cast<double>(GetTimeNanoseconds()) * 0.000000001;
}

double GetTimeMilliseconds()
{
	return static_cast<double>(GetTimeNanoseconds()) * 0.000001;
}

u64 GetTimeNanoseconds()
{
	auto time = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch()).count();
}

std::chrono::high_resolution_clock::time_point StartTimer()
{
	return std::chrono::high_resolution_clock::now();
}

double EndTimerSeconds(std::chrono::high_resolution_clock::time_point const& startTime)
{
	return static_cast<double>(EndTimerNanoseconds(startTime)) * 0.000000001;
}

u64 EndTimerMilliseconds(std::chrono::high_resolution_clock::time_point const& startTime)
{
	auto endTime = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
}

u64 EndTimerNanoseconds(std::chrono::high_resolution_clock::time_point const& startTime)
{
	auto endTime = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
}

std::thread::id GetThreadID()
{
	return std::this_thread::get_id();
}
