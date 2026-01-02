#pragma once

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
};