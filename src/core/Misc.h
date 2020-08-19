#pragma once


#include <chrono>
#include <cstdint>
#include <thread>


void *AlignedMalloc(size_t size, size_t alignment);
void AlignedFree(void *ptr);

void *InfuseData(void *arrayA, size_t strideA, size_t offsetInA,
				 void *arrayB, size_t strideB, size_t offsetInB, size_t sizeB, size_t n);

void RandomSeed(unsigned int seed);
double RandomUniform();
double RandomUniform(double from, double to);

double GetTimeSeconds();
double GetTimeMilliseconds();
std::uint64_t GetTimeNanoseconds();
std::chrono::high_resolution_clock::time_point StartTimer();
double EndTimerSeconds(std::chrono::high_resolution_clock::time_point const& startTime);
std::uint64_t EndTimerMilliseconds(std::chrono::high_resolution_clock::time_point const& startTime);
std::uint64_t EndTimerNanoseconds(std::chrono::high_resolution_clock::time_point const& startTime);

template<typename T> T Exchange(T &a, T &b)
{
	T tmp = a;
	a = b;
	return tmp;
}

std::thread::id GetThreadID();

