package com.darusc.vcamdroid.networking.connection

import android.util.Log
import java.io.InputStream
import java.io.OutputStream
import java.net.Socket
import java.util.concurrent.atomic.AtomicBoolean

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

    override val localIpAddress: String
        get() = socket.localAddress.hostAddress!!

    init {
        try {
            socket = Socket(ipAddress, port)

            outputStream = socket.getOutputStream()
            inputStream = socket.getInputStream()

            Log.d(TAG, "TCP Connected to $ipAddress:$port")

            thread = Thread { startReceiveBytesLoop() }
            thread.start()
        } catch (e: Exception) {
            throw ConnectionFailedException("$ipAddress:$port", e.message ?: "")
        }
    }

    override fun send(bytes: ByteArray) {
        socket.getOutputStream().write(bytes)
    }

    override fun send(bytes: ByteArray, size: Int) {
        socket.getOutputStream().write(bytes, 0, size)
    }

    private fun startReceiveBytesLoop() {
        val buf = ByteArray(512)
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