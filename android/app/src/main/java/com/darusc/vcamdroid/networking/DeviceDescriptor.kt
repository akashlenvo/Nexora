package com.darusc.vcamdroid.networking

import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.charset.StandardCharsets

/**
 * @param name Device name
 * @param url RTSP stream URL
 * @param backResolutions Supported video resolutions of the default back camera (main back camera, usually the wide lens)
 * @param frontResolutions Supported video resolutions of the default front camera
 */
data class DeviceDescriptor(
    val name: String,
    val url: String,
    val backResolutions: List<Pair<Int, Int>>,
    val frontResolutions: List<Pair<Int, Int>>
) {

    fun serialize(): ByteArray {
        var requiredSize = 0

        // 2 bytes needed for header (size of string) + the size of the actual string
        requiredSize += 2 + name.toByteArray(StandardCharsets.UTF_8).size
        requiredSize += 2 + url.toByteArray(StandardCharsets.UTF_8).size

        // 2 bytes needed for header (number of resolutions) +
        // for each resolution we need 4 bytes (2 for width, 2 for height)
        requiredSize += 2 + frontResolutions.size * 4
        requiredSize += 2 + backResolutions.size * 4

        val buffer = ByteBuffer.allocate(requiredSize)
        buffer.order(ByteOrder.BIG_ENDIAN)

        fun putString(s: String) {
            val bytes = s.toByteArray(StandardCharsets.UTF_8)

            buffer.putShort(bytes.size.toShort())
            buffer.put(bytes)
        }

        putString(name)
        putString(url)

        buffer.putShort(frontResolutions.size.toShort())
        frontResolutions.forEach { (w, h) ->
            buffer.putShort(w.toShort())
            buffer.putShort(h.toShort())
        }

        buffer.putShort(backResolutions.size.toShort())
        backResolutions.forEach { (w, h) ->
            buffer.putShort(w.toShort())
            buffer.putShort(h.toShort())
        }

        return buffer.array()
    }
}