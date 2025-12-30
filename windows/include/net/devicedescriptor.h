#pragma once

#include <string>
#include <vector>

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

	static DeviceDescriptor Create(const char* bytes, int size);

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

	const std::vector<Resolution>& resolutions() const
	{
		return availableResolutions;
	}

	bool operator==(const DeviceDescriptor& other) const
	{
		return this->rtspUrl == other.rtspUrl && this->deviceName == other.deviceName;
	}

private:
	DeviceDescriptor(std::string name, std::string url, std::vector<Resolution> resolutions) 
		: deviceName(name), 
		rtspUrl(url), 
		availableResolutions(resolutions) {}
	
	std::string deviceName;
	std::string rtspUrl;
	std::vector<Resolution> availableResolutions;
};