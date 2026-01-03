package com.darusc.vcamdroid.networking

import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.charset.StandardCharsets

data class ErrorReport(
    val severity: Severity,
    val error: String,
    val description: String
) {
    enum class Severity {
        WARNING,
        ERROR
    }

    fun serialize(): ByteArray {
        var requiredSize = 0

        // 1 byte for severity
        requiredSize++

        // 2 bytes needed for header (size of string) + the size of the actual string
        requiredSize += 2 + error.toByteArray(StandardCharsets.UTF_8).size
        requiredSize += 2 + description.toByteArray(StandardCharsets.UTF_8).size

        val buffer = ByteBuffer.allocate(requiredSize)
        buffer.order(ByteOrder.BIG_ENDIAN)

        fun putString(s: String) {
            val bytes = s.toByteArray(StandardCharsets.UTF_8)

            buffer.putShort(bytes.size.toShort())
            buffer.put(bytes)
        }

        buffer.put(severity.ordinal.toByte())
        putString(error)
        putString(description)

        return buffer.array()
    }
}