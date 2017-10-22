#pragma once

#include <cstdint>

struct GFXParams
{
	GFXParams(
		uint32_t iX, uint32_t iY,
		uint32_t iWidth, uint32_t iHeight,
		uint32_t iFullscreen, uint32_t iDisplayHZ, uint32_t iMultiSamples) :
		x(iX), y(iY), width(iWidth), height(iHeight), displayHZ(iDisplayHZ), multiSamples(iMultiSamples)
	{}

	GFXParams() :
		x(0), y(0), width(0), height(0), displayHZ(0), multiSamples(0) {}

	uint32_t x, y;
	uint32_t width, height;
	uint32_t fullscreen;
	uint32_t displayHZ;
	uint32_t multiSamples;
};
