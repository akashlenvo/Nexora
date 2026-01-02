#pragma once

#include "rtsp/manager.h"

/*
	State store for a connected device
*/
struct State
{
	// State registry mapping device name to its cached state store
	using Registry = std::map<std::string, State>;

	// Maps filter names to values
	using FilterValueCache = std::map<std::string, int>;
	// Maps filter category to active filter in that category
	using ActiveFilterCache = std::map<int, std::string>;

	int fps = 30;
	std::pair<int, int> resolution = { 640, 480 };
	bool backCameraActive = false;

	// Cached values of adjustment filters
	FilterValueCache filterSliderValues;
	// Cached selected filters per every effect category
	ActiveFilterCache activeFilters;

	bool adaptiveBitrate = false;
	int bitrate = RTSP::Manager::DEFAULT_STATIC_BITRATE; 
	int minBitrate = RTSP::Manager::DEFAULT_MIN_BITRATE;
	int maxBitrate = RTSP::Manager::DEFAULT_MAX_BITRATE;

	bool stabilizationEnabled = false;
	bool flashEnabled = false;
	bool h265Enabled = false;
	int focusMode = 0;
};