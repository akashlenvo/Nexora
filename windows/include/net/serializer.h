#pragma once

#include "devicedescriptor.h"
#include "rtsp/streamoptions.h"

namespace Serializer
{
	DeviceDescriptor DeserializeDeviceDescriptor(const uint8_t* bytes, size_t size);
	std::vector<uint8_t> SerializeStreamOptions(const StreamOptions& state);
}