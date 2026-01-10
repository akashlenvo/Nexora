package com.darusc.vcamdroid.rtsp

import com.darusc.vcamdroid.video.filters.FilterAdjuster
import com.darusc.vcamdroid.video.filters.FilterRepository
import com.darusc.vcamdroid.video.filters.custom.HorizontalFlipFilterRender
import com.darusc.vcamdroid.video.filters.custom.VerticalFlipFilterRender
import com.pedro.encoder.input.gl.render.filters.BaseFilterRender
import com.pedro.encoder.input.video.CameraHelper
import java.nio.ByteBuffer
import java.nio.charset.StandardCharsets

data class StreamOptions(
    var camera: CameraHelper.Facing = CameraHelper.Facing.BACK,
    var bitrate: Int = 4000 * 1024,
    var adaptiveBitrateMin: Int = 500 * 1024,
    var adaptiveBitrateMax: Int = 25000 * 1024,
    var adaptiveBitrateEnabled: Boolean = false,
    var width: Int = 640,
    var height: Int = 480,
    var fps: Int = 30,
    var rotation: Int = 0,
    var activeEffectFilter: BaseFilterRender? = null,
    var activeCorrectionFilters: MutableSet<BaseFilterRender> = HashSet(),
    var stabilization: Boolean = false,
    var focusMode: Int = 0,
    var h265Enabled: Boolean = false,
    var vFlipFilter: VerticalFlipFilterRender? = null,
    var hFlipFilter: HorizontalFlipFilterRender? = null
) {
    enum class FlipAxis { HORIZONTAL, VERTICAL }

    companion object {
        fun deserialize(data: ByteArray): StreamOptions {
            val buffer = ByteBuffer.wrap(data) // Defaults to Big Endian

            // Skip over the first byte which is packet type
            buffer.get()

            // 1. Fixed Fields
            val fps = buffer.getInt()
            val width = buffer.getInt()
            val height = buffer.getInt()

            val camera = if (buffer.get()
                    .toInt() == 1
            ) CameraHelper.Facing.BACK else CameraHelper.Facing.FRONT
            val adaptiveBitrate = buffer.get().toInt() == 1

            val bitrate = buffer.getInt()
            val minBitrate = buffer.getInt()
            val maxBitrate = buffer.getInt()

            val stabilization = buffer.get().toInt() == 1
            val flash = buffer.get().toInt() == 1
            val h265 = buffer.get().toInt() == 1

            val focusMode = buffer.getInt()

            fun readString(): String {
                val len = buffer.short.toInt() and 0xFFFF // Unsigned short
                val bytes = ByteArray(len)
                buffer.get(bytes)
                return String(bytes, StandardCharsets.UTF_8)
            }

            // 2. Filter Sliders Map
            val filterCount = buffer.short.toInt() and 0xFFFF
            val filtersMap = mutableSetOf<BaseFilterRender>()
            repeat(filterCount) {
                val fname = readString()
                val fvalue = buffer.getInt()
                FilterRepository.create(fname)?.let {
                    FilterAdjuster.adjust(it, fvalue)
                    filtersMap.add(it)
                }
            }

            // 3. Active Filters Map
            val activeEffectFilter = readString()

            return StreamOptions(
                camera,
                bitrate,
                minBitrate,
                maxBitrate,
                adaptiveBitrate,
                width,
                height,
                fps,
                0,
                FilterRepository.create(activeEffectFilter),
                filtersMap,
                stabilization,
                focusMode,
                h265,
            )
        }
    }
}