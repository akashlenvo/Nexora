#pragma once

#include <tuple>
#include <thread>
#include <asio.hpp>
#include <string>

#include "connection.h"

class Server : std::enable_shared_from_this<Server>
{
public:
	using tcp = asio::ip::tcp;
	using udp = asio::ip::udp;
	using HostInfo = std::tuple<std::string, std::string, std::string>;

	struct PacketType
	{
		static const int FRAME = 0x00;
		static const int RESOLUTION = 0x01;
		static const int ACTIVATION = 0x02;
		static const int CAMERA = 0x03;
		static const int QUALITY = 0x04;
		static const int WB = 0x05;
		static const int EFFECT = 0x06;
	};

	struct ConnectionListener
	{
		virtual void OnDeviceConnected(DeviceDescriptor& descriptor) const = 0;
		virtual void OnDeviceDisconnected(DeviceDescriptor& descriptor) const = 0;
	};

	struct DeviceInfo
	{
		std::string name;
		std::string address;
		unsigned short port;
	};

	Server(int port, const ConnectionListener& connectionListener);

	/// <summary>
	/// Gets host device's info (name, IPv4 address and port)
	/// </summary>
	HostInfo GetHostInfo();

	void Start();
	void Close();

	/*void SetStreamResolution(unsigned short width, unsigned short height);
	void SetStreamingCamera(bool back = true);
	void SetStreamingQuality(uint8_t quality);
	void SetStreamingWB(int wb);
	void SetStreamingEffect(int effect);*/

private:
	int port;

	const ConnectionListener& connectionListener;

	//asio::executor_work_guard<asio::io_context::executor_type> guard;
	asio::io_context context;

	tcp::acceptor acceptor;
	udp::endpoint remote_endpoint;

	std::thread thread;

	int streamingDevice;
	std::vector<std::shared_ptr<Connection>> connections;

	void TCPDoAccept();
	void OnConnectionDisconnected(std::shared_ptr<Connection> connection);
};