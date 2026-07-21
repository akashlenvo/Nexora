#pragma once

#include "devicedescriptor.h"
#include "rtsp/streamoptions.h"
#include "net/connection.h"
#include <stdexcept>

namespace Serializer
{
	class IncompletePacket : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
	};

	DeviceDescriptor DeserializeDeviceDescriptor(const uint8_t* bytes, size_t size);
	std::vector<uint8_t> SerializeStreamOptions(const StreamOptions& state);
	Connection::ErrorReport DeserializeErrorReport(const uint8_t* bytes, size_t size);
}
