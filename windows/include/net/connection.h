#pragma once

#include <memory>
#include <functional>
#include <asio.hpp>

#include "devicedescriptor.h"

/// <summary>
/// Manages a TCP connection
/// </summary>
class Connection : public std::enable_shared_from_this<Connection>
{
	friend class Server;

public:
	using tcp = asio::ip::tcp;
	using udp = asio::ip::udp;
	using OnDisconnectedListener = std::function<void(std::shared_ptr<Connection>)>;

	Connection(tcp::socket socket, DeviceDescriptor& descriptor, OnDisconnectedListener onDisconnectedListener);
private:

	OnDisconnectedListener onDisconnectedListener;

	tcp::socket socket;
	unsigned char* byteBuffer;

	bool active;
	DeviceDescriptor descriptor;

	/// <summary>
	/// Connected devices only send their name once after connecting.
	/// That is handled directly by the server when creating a new connection.
	/// This function is used only to detect when a device disconnects.
	/// </summary>
	void Read();

	void Send(std::string message);
	void Send(const unsigned char* bytes, size_t size);
	void Close(bool stoppedByServer = false);
};
