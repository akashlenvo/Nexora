#include "net/server.h"

#include "net/devicedescriptor.h"
#include "logger.h"
#include "adb.h"

Server::Server(int port, const ConnectionListener& connectionListener) :
	port(port),
	connectionListener(connectionListener),
	acceptor(tcp::acceptor(context, tcp::endpoint(tcp::v4(), port)))
{
	streamingDevice = 0;
	adb::reverse(port);
	adb::forward(8554);
}

Server::HostInfo Server::GetHostInfo()
{
	std::string name = asio::ip::host_name();

	// Get the active network adapter by 
	// creating a dummy UDP connection and let the OS 
	// determine what network adapter will be used
	udp::resolver resolver(context);
	udp::socket socket(context);

	socket.connect(udp::endpoint(asio::ip::address::from_string("8.8.8.8"), 80));

	// The IPv4 address needed is the address of the local endpoint assigned to the socket
	// (the network adapter actual traffic is routed trough)
	asio::ip::address local_addr = socket.local_endpoint().address();

	socket.close();

	return { name, local_addr.to_string(), std::to_string(port)};
}

void Server::Start()
{
	try
	{
		TCPDoAccept();
		thread = std::thread([this]() {
			context.run();
		});
		logger << "[SERVER] Started" << std::endl;
	}
	catch (std::exception e)
	{
		logger << "[SERVER] Start failed\n";
	}
}

void Server::Close()
{
	logger << "[SERVER] Closing...\n";

	acceptor.close();
	
	for (std::shared_ptr<Connection> conn : connections)
	{
		conn->Close(true);
	}

	context.stop();

	if (thread.joinable())
	{
		thread.join();
	}

	logger << "[SERVER] Closed.\n";

	adb::kill(port);
}

/*void Server::SetStreamResolution(unsigned short width, unsigned short height)
{
	unsigned char bytes[5] = {
		PacketType::RESOLUTION,							// First byte is the packet type
		static_cast<unsigned char>(width & 0xFF),		// Low byte of width
		static_cast<unsigned char>(width >> 8),			// High byte of width
		static_cast<unsigned char>(height & 0xFF),		// Low byte of height
		static_cast<unsigned char>(height >> 8),		// High byte of height
	};

	for (auto& conn : connections)
	{
		conn->Send(bytes, 5);
	}
}

void Server::SetStreamingDevice(int device)
{
	if (device < 0 || device >= connections.size())
		return;

	unsigned char startBytes[2] = { PacketType::ACTIVATION, 0x01 };
	unsigned char stopBytes[2] = { PacketType::ACTIVATION, 0x00 };

	connections[device]->Send(startBytes, 2);
	connections[device]->active = true;
	
	// Reset();
	logger << "[SERVER] Set UDP stream device: " << connections[device]->socket.remote_endpoint() << std::endl;

	for (int i = 0; i < connections.size(); i++)
	{
		if(i != device)
		{
			connections[i]->active = false;
			connections[i]->Send(stopBytes, 2);
		}
	}
}

void Server::SetStreamingCamera(bool back)
{
	unsigned char bytes[2] = { PacketType::CAMERA, back ? 0x01 : 0x00 };
	for (auto& conn : connections)
	{
		conn->Send(bytes, 2);
	}
}

void Server::SetStreamingQuality(uint8_t quality)
{
	unsigned char bytes[2] = { PacketType::QUALITY, quality };
	for (auto& conn : connections)
	{
		conn->Send(bytes, 2);
	}
}

void Server::SetStreamingWB(int wb)
{
	unsigned char bytes[2] = { PacketType::WB, wb };
	for (auto& conn : connections)
	{
		conn->Send(bytes, 2);
	}
}

void Server::SetStreamingEffect(int effect)
{
	unsigned char bytes[2] = { PacketType::EFFECT, effect };
	for (auto& conn : connections)
	{
		conn->Send(bytes, 2);
	}
}

int Server::GetStreamingDevice()
{
	for (int i = 0; i < connections.size(); i++)
	{
		if (connections[i]->active)
		{
			return i;
		}
	}
	return -1;
}*/

void Server::TCPDoAccept()
{
	acceptor.async_accept([&, this](asio::error_code ec, tcp::socket socket) {
		if (!ec)
		{
			logger << "[SERVER] Device connected." << socket.remote_endpoint() << std::endl;

			// Read the only message sent by the client (the device descriptor)
			// We need to read this here because the connection listener
			// needs all the data sent by the client (trough GetConnectedDevicesInfo)
			// otherwise the connection would need to be passed to the connection
			std::array<char, 512> buffer;
			size_t size = socket.read_some(asio::buffer(buffer, 512));
			
			// auto ipaddress = socket.remote_endpoint().address().to_string();
			auto descriptor = DeviceDescriptor::Create(buffer.data(), size);

			auto conn = std::make_shared<Connection>(std::move(socket), descriptor, std::bind(&Server::OnConnectionDisconnected, this, std::placeholders::_1));
			connections.push_back(std::move(conn));

			connectionListener.OnDeviceConnected(descriptor);
		}

		TCPDoAccept();
	});
}

void Server::OnConnectionDisconnected(std::shared_ptr<Connection> connection)
{
	connections.erase(std::remove(connections.begin(), connections.end(), connection), connections.end());
	connectionListener.OnDeviceDisconnected(connection->descriptor);
}
