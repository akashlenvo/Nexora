package com.darusc.vcamdroid.networking

import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.charset.StandardCharsets

data class DeviceDescriptor(
    val name: String,
    val url: String,
    val resolutions: List<Pair<Int, Int>>
) {

    fun serialize(): ByteArray {
        var requiredSize = 0

        // 2 bytes needed for header (size of string) + the size of the actual string
        requiredSize += 2 + name.toByteArray(StandardCharsets.UTF_8).size
        requiredSize += 2 + url.toByteArray(StandardCharsets.UTF_8).size

        // 2 bytes needed for header (number of resolutions) +
        // for each resolution we need 4 bytes (2 for width, 2 for height)
        requiredSize += 2 + resolutions.size * 4

        val buffer = ByteBuffer.allocate(requiredSize)
        buffer.order(ByteOrder.BIG_ENDIAN)

        fun putString(s: String) {
            val bytes = s.toByteArray(StandardCharsets.UTF_8)

            buffer.putShort(bytes.size.toShort())
            buffer.put(bytes)
        }

        putString(name)
        putString(url)

        buffer.putShort(resolutions.size.toShort())
        resolutions.forEach { (w, h) ->
            buffer.putShort(w.toShort())
            buffer.putShort(h.toShort())
        }

        return buffer.array()
    }
}