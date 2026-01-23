package com.darusc.vcamdroid.networking.connection

import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress

/**
 * Wrapper around a socket to manage the TCP connection.
 * Using it only to send the video stream. No packet receiving implementation.
 */
/*class UDPConnection(
    ipAddress: String,
    private val port: Int,
) : Connection() {

    private val address: InetAddress = InetAddress.getByName(ipAddress)

    override val maxPacketSize = 65472

    private var socket: DatagramSocket

    init {
        try {
            socket = DatagramSocket()
            socket.connect(address, port)
            socket.sendBufferSize = 921600

            Log.d(TAG, "UDP Connected to $ipAddress:$port")
        } catch (e: Exception) {
            throw ConnectionFailedException("$ipAddress:$port")
        }
    }

    override fun send(bytes: ByteArray) {
        try {
            socket.send(DatagramPacket(bytes, bytes.size, address, port))
            //socket.receive(DatagramPacket(ByteArray(5), 5, address, port))
        } catch (_: Exception) {}
    }

    override fun send(bytes: ByteArray, size: Int) {
        try {
            socket.send(DatagramPacket(bytes, size, address, port))
            //socket.receive(DatagramPacket(ByteArray(5), 5, address, port))
        } catch (_: Exception) {}
    }

    override fun close() {
        socket.close()
    }
}*/