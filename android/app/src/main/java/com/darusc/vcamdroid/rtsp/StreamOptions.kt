package com.darusc.vcamdroid.rtsp

import com.pedro.encoder.input.video.CameraHelper

data class StreamOptions(
    var camera: CameraHelper.Facing,
    var bitrate: Int,
    var width: Int,
    var height: Int,
    var fps: Int,
    var rotation: Int
) {
    companion object {
        fun default(): StreamOptions {
            return StreamOptions(CameraHelper.Facing.BACK, 4000 * 1024, 640, 480, 30, 0)
        }
    }
}