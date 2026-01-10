#include "net/serializer.h"

#include <string>

namespace Serializer
{
	static uint16_t ReadInt16(const uint8_t* bytes)
	{
		uint8_t b0 = static_cast<uint8_t>(bytes[0]);
		uint8_t b1 = static_cast<uint8_t>(bytes[1]);

		return (b0 << 8) | b1;
	}

	static void WriteInt16(std::vector<uint8_t>& buffer, uint16_t value)
	{
		buffer.push_back((value >> 8) & 0xFF);
		buffer.push_back(value & 0xFF);
	}

	static void WriteInt32(std::vector<uint8_t>& buffer, uint32_t value)
	{
		buffer.push_back((value >> 24) & 0xFF);
		buffer.push_back((value >> 16) & 0xFF);
		buffer.push_back((value >> 8) & 0xFF);
		buffer.push_back(value & 0xFF);
	}

	static void WriteBool(std::vector<uint8_t>& buffer, bool value)
	{
		WriteInt32(buffer, value);
	}

	static void WriteString(std::vector<uint8_t>& buffer, std::string value)
	{
		WriteInt16(buffer, static_cast<uint16_t>(value.size()));
		buffer.insert(buffer.end(), value.begin(), value.end());
	}

	DeviceDescriptor DeserializeDeviceDescriptor(const uint8_t* bytes, size_t size)
	{
		int offset = 0;

		// Name is the first thing
		// Each string is preceded by 2 bytes representing its size
		auto name = std::string((const char*)& bytes[offset + 2], ReadInt16(bytes));

		// The RTSP url follows after the name
		offset += 2 + name.size();
		auto url = std::string((const char*)& bytes[offset + 2], ReadInt16(bytes + offset));

		// Default protocol is udp. If it is a usb connection (via adb) switch to tcp
		std::string protocol = "udp";
		if (url.find("127.0.0.1") != std::string::npos)
		{
			protocol = "tcp";
		}

		offset += 2 + url.size();
		// Represents how many resolutions are provided
		uint16_t frontResolutionCount = ReadInt16(bytes + offset);
		offset += 2;

		std::vector<DeviceDescriptor::Resolution> frontResolutions;
		for (int i = 0; i < frontResolutionCount; i++)
		{
			// Each resolutions is given in pairs, first 2 bytes represents 
			// the width, the next 2 bytes represents the height
			auto w = ReadInt16(bytes + offset);
			auto h = ReadInt16(bytes + offset + 2);

			frontResolutions.push_back(DeviceDescriptor::Resolution(w, h));

			offset += 4;
		}

		uint16_t backResolutionCount = ReadInt16(bytes + offset);
		offset += 2;

		std::vector<DeviceDescriptor::Resolution> backResolutions;
		for (int i = 0; i < backResolutionCount; i++)
		{
			// Each resolutions is given in pairs, first 2 bytes represents 
			// the width, the next 2 bytes represents the height
			auto w = ReadInt16(bytes + offset);
			auto h = ReadInt16(bytes + offset + 2);

			backResolutions.push_back(DeviceDescriptor::Resolution(w, h));

			offset += 4;
		}

		// How many filters
		uint16_t filterCount = ReadInt16(bytes + offset);
		offset += 2;

		Video::Filter::Registry filters;
		for (int i = 0; i < filterCount; i++)
		{
			// First 2 bytes represent the length of the filter name,
			// followed by the actual string
			auto name = std::string((const char*)&bytes[offset + 2], ReadInt16(bytes + offset));
			offset += 2 + name.size();
			// And 1 more byte for the category
			auto cat = static_cast<Video::Filter::Category>(bytes[offset]);
			offset++;

			filters[cat].push_back(name);
		}

		return DeviceDescriptor(name, url, protocol, frontResolutions, backResolutions, filters);
	}

	std::vector<uint8_t> SerializeStreamOptions(const StreamOptions& state)
	{
		std::vector<uint8_t> buffer;
		buffer.reserve(512);

		// First byte of the buffer is reserved
		buffer.push_back(0);

		WriteInt32(buffer, state.fps);
		WriteInt32(buffer, state.resolution.first);
		WriteInt32(buffer, state.resolution.second);

		WriteBool(buffer, state.backCameraActive);
		
		WriteBool(buffer, state.adaptiveBitrate);
		WriteInt32(buffer, state.bitrate);
		WriteInt32(buffer, state.minBitrate);
		WriteInt32(buffer, state.maxBitrate);

		WriteBool(buffer, state.stabilizationEnabled);
		WriteBool(buffer, state.flashEnabled);
		WriteBool(buffer, state.h265Enabled);

		WriteInt16(buffer, static_cast<uint16_t>(state.filterSliderValues.size()));
		for (const auto& [name, value] : state.filterSliderValues)
		{
			WriteString(buffer, name);
			WriteInt32(buffer, value);
		}

		WriteString(buffer, state.activeEffectFilter);

		return buffer;
	}

	Connection::ErrorReport DeserializeErrorReport(const uint8_t* bytes, size_t size)
	{
		int offset = 0;

		Connection::ErrorReport report;

		report.severity = bytes[offset++];
		report.error = std::string((const char*)&bytes[offset + 2], ReadInt16(bytes + offset));

		offset += 2 + report.error.size();
		report.description = std::string((const char*)&bytes[offset + 2], ReadInt16(bytes + offset));
		
		return report;
	}
}