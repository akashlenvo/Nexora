#include "net/server.h"

#include "logger.h"
#include "adb.h"
#include "net/serializer.h"

Server::Server(int port, const ConnectionListener& connectionListener) :
	port(port),
	connectionListener(connectionListener),
	acceptor(tcp::acceptor(context, tcp::endpoint(tcp::v4(), port)))
{
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

void Server::Send(int id, const unsigned char* bytes, size_t size) const
{
	if (id >= 0 && id < connections.size())
	{
		connections[id]->Send(bytes, size);
	}
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
			auto descriptor = Serializer::DeserializeDeviceDescriptor((const uint8_t*)buffer.data(), size);

			auto conn = std::make_shared<Connection>(
				std::move(socket), 
				descriptor, 
				std::bind(&Server::OnConnectionDisconnected, this, std::placeholders::_1),
				std::bind(&Server::OnConnectionReportingError, this, std::placeholders::_3)
			);
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

void Server::OnConnectionReportingError(std::shared_ptr<Connection> connection, const uint8_t* bytes, size_t size)
{
	auto report = Serializer::DeserializeErrorReport(bytes, size);
	connectionListener.OnDeviceErrorReported(connection->descriptor, report);
}
