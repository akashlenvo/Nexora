package com.darusc.vcamdroid.networking.connection

abstract class Connection {

    protected val TAG = "VCamdroid"

    abstract val maxPacketSize: Int

    class ConnectionFailedException(host: String, reason: String) : Exception("Connection to $host failed! Reason: $reason")

    interface Listener {
        fun onBytesReceived(buffer: ByteArray, bytes: Int)
        fun onDisconnected()
    }

    abstract val localIpAddress: String

    abstract fun send(bytes: ByteArray)
    abstract fun send(bytes: ByteArray, size: Int)
    abstract fun close()
}