package com.darusc.vcamdroid.rtsp

import com.pedro.encoder.input.gl.render.filters.BaseFilterRender
import com.pedro.encoder.input.video.CameraHelper

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
    var activeCorrectionFilters: MutableSet<BaseFilterRender> = HashSet()
) {
    companion object {
        fun default(): StreamOptions {
            return StreamOptions()
        }
    }
}