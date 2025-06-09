#pragma once

#include <tuple>
#include <thread>
#include <asio.hpp>
#include <string>

#include "connection.h"
#include "receiver.h"

/// <summary>
/// Server implementation for the custom transmission protocol in use.
///
/// The server accepts tcp connections in order to be able to manage and
/// control connected streaming devices.
///
/// Messaging between server and clients is done over TCP while actual
/// video streaming is done over UDP. The server commands which client
/// should stream video feed.
/// </summary>
class Server : std::enable_shared_from_this<Server>, public Receiver
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
		virtual void OnDeviceConnected(std::string device) const = 0;
		virtual void OnDeviceDisconnected(std::string device) const = 0;
	};

	struct DeviceInfo
	{
		std::string name;
		std::string address;
		unsigned short port;
	};

	Server(int port, const ConnectionListener& connectionListener, const Receiver::FrameReceivedListener& bytesReceivedListener);

	/// <summary>
	/// Gets host device's info (name, IPv4 address and port)
	/// </summary>
	HostInfo GetHostInfo();

	void Start();
	void Close();

	void SetStreamResolution(unsigned short width, unsigned short height);
	void SetStreamingDevice(int device);
	void SetStreamingCamera(bool back = true);
	void SetStreamingQuality(uint8_t quality);
	void SetStreamingWB(int wb);
	void SetStreamingEffect(int effect);

	int GetStreamingDevice();

	std::vector<DeviceInfo> GetConnectedDevicesInfo();

private:
	int port;

	const ConnectionListener& connectionListener;

	//asio::executor_work_guard<asio::io_context::executor_type> guard;
	asio::io_context context;

	tcp::acceptor acceptor;
	udp::socket udpsocket;
	udp::endpoint remote_endpoint;

	std::thread thread;

	int streamingDevice;
	std::vector<std::shared_ptr<Connection>> connections;

	void TCPDoAccept();
	void StartReceive();
	void OnConnectionDisconnected(std::shared_ptr<Connection> connection);
};