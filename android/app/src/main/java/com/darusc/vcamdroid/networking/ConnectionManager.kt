package com.darusc.vcamdroid.networking

import android.util.Log
import androidx.camera.core.processing.Packet
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.io.InputStream
import java.io.OutputStream
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress
import java.net.Socket
import java.util.concurrent.atomic.AtomicBoolean
import kotlin.experimental.and
import kotlin.experimental.or
import kotlin.math.ceil
import kotlin.math.min

class ConnectionManager private constructor() : Connection.Listener {

    private val TAG = "VCamdroid"

    companion object {
        @Volatile private var instance: ConnectionManager? = null

        fun getInstance(): ConnectionManager {
            synchronized(this) {
                return instance ?: ConnectionManager().also { instance = it }
            }
        }

        fun getInstance(connectionStateCallback: ConnectionStateCallback): ConnectionManager {
            synchronized(this) {
                if(instance == null) {
                    instance = ConnectionManager()
                }
                instance!!.setConnectionStateCallback(connectionStateCallback)
                return instance!!
            }
        }
    }

    enum class Mode {
        USB,
        WIFI
    }

    class PacketType {
        companion object {
            const val FRAME: Byte = 0x00
            const val RESOLUTION: Byte = 0x01
            const val ACTIVATION: Byte = 0x02
            const val CAMERA: Byte = 0x03
            const val QUALITY: Byte = 0x04
            const val WB: Byte = 0x05
            const val EFFECT: Byte = 0x06
        }
    }

    private var connectionStateCallback: ConnectionStateCallback? = null
    private var onBytesReceivedCallback: ((buffer: ByteArray, bytes: Int) -> Unit)? = null

    private var tcpConn: TCPConnection? = null
    private var udpConn: UDPConnection? = null

    /**
     * Active connection. Prioritize UDP connection if available
     * otherwise fall back to the TCP Conneciton
     */
    private val connection
        get() = (udpConn ?: tcpConn) as Connection

    private var streamingEnabled = false

    interface ConnectionStateCallback {
        fun onConnectionSuccessful(connectionMode: Mode) { }
        fun onConnectionFailed(connectionMode: Mode) { }
        fun onDisconnected() { }
    }

    /**
     * Wrapper around a socket to manage the TCP connection.
     * Using it only to send the video stream. No packet receiving implementation.
     */
    private inner class UDPConnection(
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
    }

    /**
     * Wrapper around a socket to manage the TCP connection
     * @param ipAddress Remote endpoint's IPv4 address
     * @param port Remote endpoint's port
     * @param listener The registered listener for callbacks
     */
    class TCPConnection(
        ipAddress: String,
        port: Int,
        private val isOverAdb: Boolean,
        private val listener: Connection.Listener
    ) : Connection() {

        override val maxPacketSize = 65472

        private var socket: Socket
        private var outputStream: OutputStream? = null
        private var inputStream: InputStream? = null

        private var thread: Thread
        private val running = AtomicBoolean(true)

        init {
            try {
                socket = Socket(ipAddress, port)

                outputStream = socket.getOutputStream()
                inputStream = socket.getInputStream()

                Log.d(TAG, "TCP Connected to $ipAddress:$port")

                thread = Thread { startReceiveBytesLoop() }
                thread.start()
            } catch (e: Exception) {
                throw ConnectionFailedException("$ipAddress:$port")
            }
        }

        override fun send(bytes: ByteArray) {
            socket.getOutputStream().write(bytes)
        }

        override fun send(bytes: ByteArray, size: Int) {
            socket.getOutputStream().write(bytes, 0, size)
        }

        private fun startReceiveBytesLoop() {
            val buf = ByteArray(15)
            while (running.get()) {
                try {
                    val bytes = inputStream?.read(buf)
                    // When connected over ADB stream end of file might be reached
                    if(isOverAdb && bytes == -1) {
                        listener.onDisconnected()
                        break
                    }
                    listener.onBytesReceived(buf, bytes ?: 0)
                } catch (e: Exception) {
                    Log.e(TAG, "Error while reading. Closing socket. ${e.message}")
                    listener.onDisconnected()
                    break
                }
            }
        }

        override fun close() {
            running.set(false)
            socket.close()
            thread.join()
        }
    }

    private fun setConnectionStateCallback(connectionStateCallback: ConnectionStateCallback) {
        this.connectionStateCallback = connectionStateCallback
    }

    /**
     * The bytesReceivedListener is called only for packets of type different than PacketType.ACTIVATION.
     * These packets are handled internally
     */
    fun setOnBytesReceivedCallback(onBytesReceivedCallback: (buffer: ByteArray, bytes: Int) -> Unit) {
        this.onBytesReceivedCallback = onBytesReceivedCallback
    }

    /**
     * Connect in WIFI mode.
     * Tcp socket is used only for commands, video frames are streamed using Udp
     */
    fun connect(ipAddress: String, port: Int) {
        CoroutineScope(Dispatchers.IO).launch {
            try {
                tcpConn = TCPConnection(ipAddress, port, false, this@ConnectionManager)
                udpConn = UDPConnection(ipAddress, port)
                tcpConn!!.send(android.os.Build.MODEL.toByteArray())
                connectionStateCallback?.onConnectionSuccessful(Mode.WIFI)
            } catch (e: Connection.ConnectionFailedException) {
                Log.e(TAG, "Connection manager: ${e.message}")
                connectionStateCallback?.onConnectionFailed(Mode.WIFI)
            }
        }
    }

    /**
     * Connect in USB mode (through adb)
     * Tcp socket is used both for connection and streaming (adb doesn't allow Udp port forwarding)
     */
    fun connect(port: Int) {
        CoroutineScope(Dispatchers.IO).launch {
            try {
                tcpConn = TCPConnection("127.0.0.1", port, true, this@ConnectionManager)
                tcpConn!!.send(android.os.Build.MODEL.toByteArray())
                connectionStateCallback?.onConnectionSuccessful(Mode.USB)
            } catch (e: Connection.ConnectionFailedException) {
                Log.e(TAG, "Connection manager: ${e.message}")
                connectionStateCallback?.onConnectionFailed(Mode.USB)
            }
        }
    }

    fun sendVideoStreamFrame(frame: ByteArray, withCoroutine: Boolean = false) {
        if (!streamingEnabled || connection == null) {
            return
        }

        when (withCoroutine) {
            false -> sendVideoStreamFrame(frame)
            true -> CoroutineScope(Dispatchers.IO).launch { sendVideoStreamFrame(frame) }
        }
    }

    private fun sendVideoStreamFrame(frame: ByteArray) {
        // Split the frame in n segments of size maxPacketSize
        // to send over the active connection
        var segments = ceil(frame.size / (connection.maxPacketSize.toDouble() - 3)).toInt()

        // Packet to send of size maxPacketSize
        // First byte is the packet type
        // Second byte is the current frame segment number
        // Third byte is the total number of segments
        val packet = ByteArray(connection.maxPacketSize)
        packet[0] = PacketType.FRAME
        packet[2] = segments.toByte()

        var start = 0
        for (i in 1..segments) {
            packet[1] = i.toByte()

            // Copy the segment section into the packet from frame bytes
            val end = min(frame.size, start + connection.maxPacketSize - 3)
            val size = end - start
            System.arraycopy(frame, start, packet, 3, size)

            connection.send(packet, size + 3)
            start = end
        }
    }

    override fun onBytesReceived(buffer: ByteArray, bytes: Int) {
        val packetType = buffer[0]
        when(packetType) {
            PacketType.ACTIVATION -> streamingEnabled = buffer[1] == (0x01.toByte())
            else -> onBytesReceivedCallback?.let { it(buffer, bytes) }
        }
    }

    override fun onDisconnected() {
        CoroutineScope(Dispatchers.Main).launch {
            streamingEnabled = false
            udpConn?.close()
            tcpConn?.close()
            connectionStateCallback?.onDisconnected()
        }
    }
}