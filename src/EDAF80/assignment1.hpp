#pragma once

struct SpinConfiguration
{
	float const inclination{0.0f}; // in rad
	float const speed{0.0f};       // in rad/s
};

struct OrbitConfiguration
{
	float const radius{0.0f};
	float const inclination{0.0f}; // in rad
	float const speed{0.0f};       // in rad/s
};
