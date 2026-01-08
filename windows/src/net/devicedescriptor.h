#pragma once

#include <string>
#include <vector>

#include "video/filter.h"

/*
	Describes information about a RTSP streaming device.
	
	The byte format of the device descriptor is as follows:
	each string is preceded by a 2 byte value representing its length,
	and the resolutions are preceded by a 2 byte value representing their count

	[uint16_t size][uint8_t[] name]
	[uint16_t size][uint8_t[] url]
	[uint16_t count]
	[uint16_t width][uint_16_t height] [0]
	...
	...
	...
	[uint16_t width][uint_16_t height] [count-1]
*/
struct DeviceDescriptor
{
	/*
		first = width
		second = height
	*/
	using Resolution = std::pair<int, int>;

	DeviceDescriptor(std::string name, std::string url, std::string protocol, std::vector<Resolution> frontResolutions, std::vector<Resolution> backResolutions, Video::Filter::Registry filters)
		: deviceName(name),
		rtspUrl(url),
		rtspProtocol(protocol),
		availableFrontResolutions(frontResolutions),
		availableBackResolutions(backResolutions),
		availableFilters(filters) {}

	DeviceDescriptor(const DeviceDescriptor&) = default;
	DeviceDescriptor& operator=(const DeviceDescriptor&) = default;
	DeviceDescriptor(DeviceDescriptor&&) = default;
	DeviceDescriptor& operator=(DeviceDescriptor&&) = default;

	const std::string& name() const
	{
		return deviceName;
	}

	const std::string& url() const
	{
		return rtspUrl;
	}

	const std::string& protocol() const
	{
		return rtspProtocol;
	}

	const std::vector<Resolution>& frontResolutions() const
	{
		return availableFrontResolutions;
	}

	const std::vector<Resolution>& backResolutions() const
	{
		return availableBackResolutions;
	}

	const Video::Filter::Registry& filters() const
	{
		return availableFilters;
	}

	bool operator==(const DeviceDescriptor& other) const
	{
		return this->rtspUrl == other.rtspUrl && this->deviceName == other.deviceName;
	}

private:
	std::string deviceName;
	std::string rtspUrl;
	std::string rtspProtocol;
	std::vector<Resolution> availableFrontResolutions;
	std::vector<Resolution> availableBackResolutions;
	Video::Filter::Registry availableFilters;
};