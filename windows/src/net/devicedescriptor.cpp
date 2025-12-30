#include "net/devicedescriptor.h"

uint16_t ReadInt16(const char* bytes)
{
	uint8_t b0 = static_cast<uint8_t>(bytes[0]);
	uint8_t b1 = static_cast<uint8_t>(bytes[1]);

	return (b0 << 8) | b1;
}

DeviceDescriptor DeviceDescriptor::Create(const char* bytes, int size)
{
	int offset = 0;

	// Name is the first thing
	// Each string is preceded by 2 bytes representing its size
	auto name = std::string(&bytes[offset + 2], ReadInt16(bytes));

	// The RTSP url follows after the name
	offset += 2 + name.size();
	auto url = std::string(&bytes[offset + 2], ReadInt16(bytes + offset));

	// Default protocol is udp. If it is a usb connection (via adb) switch to tcp
	std::string protocol = "udp";
	if (url.find("127.0.0.1") != std::string::npos)
	{
		protocol = "tcp";
	} 

	offset += 2 + url.size();
	// Represents how many resolutions are provided
	uint16_t resolutionCount = ReadInt16(bytes + offset);
	offset += 2;

	std::vector<DeviceDescriptor::Resolution> resolutions;
	for (int i = 0; i < resolutionCount; i++)
	{
		// Each resolutions is given in pairs, first 2 bytes represents 
		// the width, the next 2 bytes represents the height
		auto w = ReadInt16(bytes + offset);
		auto h = ReadInt16(bytes + offset + 2);

		resolutions.push_back(DeviceDescriptor::Resolution(w, h));
		
		offset += 4;
	}


	return DeviceDescriptor(name, url, protocol, resolutions);
}