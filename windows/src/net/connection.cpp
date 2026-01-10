#include "net/connection.h"

#include "logger.h"

Connection::Connection(tcp::socket socket, DeviceDescriptor& descriptor, OnDisconnectedListener onDisconnectedListener, OnBytesReceived onBytesReceived) 
	: socket(std::move(socket)),
	descriptor(descriptor),
	onDisconnectedListener(onDisconnectedListener),
	onBytesReceived(onBytesReceived)
{
	byteBuffer = new unsigned char[255];
	active = false;
	Read();
}

void Connection::Read()
{
	socket.async_read_some(asio::buffer(byteBuffer, 255), [this](asio::error_code ec, size_t bytes) {
		if (!ec)
		{
			onBytesReceived(this->shared_from_this(), (const uint8_t*)byteBuffer, bytes);
			Read();
		}
		else
		{
			// When the connection is closed by the server this function may still be called
			// and then the socket will be invalid so wrap it in a try/catch
			// 
			// Also the close() method is called inside the try to avoid closing
			// the socket again when the server is stopped
			try {
				logger << "[CONNECTION@" << this->socket.remote_endpoint() << "] Closed with error code: " << ec << std::endl;
				Close();
			} catch(std::exception e) { }
		}
	});
}

void Connection::Send(std::string message)
{
	socket.send(asio::buffer(message));
}

void Connection::Send(const unsigned char* bytes, size_t size)
{
	socket.send(asio::buffer(bytes, size));
}

void Connection::Close(bool stoppedByServer)
{
	if (stoppedByServer)
	{
		// Sometimes socket.cancel() makes the app to not respond
		// but sometimes is required to successfully remove the adb
		// port forwarding
		// TODO...
		//socket.cancel();
		socket.close();
	}
	else
	{
		socket.close();
		onDisconnectedListener(this->shared_from_this());
	}
}