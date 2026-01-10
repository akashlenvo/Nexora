package com.darusc.vcamdroid.networking

class PacketType {
    companion object {
        const val FRAME: Byte = 0x00
        const val RESOLUTION: Byte = 0x01
        const val ACTIVATION: Byte = 0x02
        const val CAMERA: Byte = 0x03
        const val QUALITY: Byte = 0x04
        const val CORRECTION_FILTER: Byte = 0x05
        const val EFFECT_FILTER: Byte = 0x06
        const val ROTATION: Byte = 0x07
        const val BITRATE: Byte = 0x08
        const val ADAPTIVE_BITRATE: Byte = 0x09
        const val STABILIZATION: Byte = 0xA
        const val FLASH: Byte = 0xB
        const val FOCUS: Byte = 0xC
        const val CODEC: Byte = 0xD
        const val FPS: Byte = 0xE
        const val ZOOM: Byte = 0xF
        const val FLIP: Byte = 0x10
    }
}