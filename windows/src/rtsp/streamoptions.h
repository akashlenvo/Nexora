#pragma once

#include "constants.h"

/*
	State store for a connected device
*/
struct StreamOptions
{
	// State registry mapping device name to its cached state store
	using Registry = std::map<std::string, StreamOptions>;

	// Maps filter names to values
	using FilterValueCache = std::map<std::string, int>;

	int fps = 30;
	std::pair<int, int> resolution = { 640, 480 };
	bool backCameraActive = true;

	// Cached values of adjustment filters
	FilterValueCache filterSliderValues;
	// Only 1 effect filter is permitted
	std::string activeEffectFilter;

	bool adaptiveBitrate = false;
	int bitrate = RTSP::DEFAULT_STATIC_BITRATE; 
	int minBitrate = RTSP::DEFAULT_MIN_BITRATE;
	int maxBitrate = RTSP::DEFAULT_MAX_BITRATE;

	float zoom = 1.0f;
	bool stabilizationEnabled = false;
	bool flashEnabled = false;
	bool h265Enabled = false;
	int focusMode = 0;
};