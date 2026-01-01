package com.darusc.vcamdroid.networking

class PacketType {
    companion object {
        const val FRAME: Byte = 0x00
        const val RESOLUTION: Byte = 0x01
        const val ACTIVATION: Byte = 0x02
        const val CAMERA: Byte = 0x03
        const val QUALITY: Byte = 0x04
        const val WB: Byte = 0x05
        const val FILTER: Byte = 0x06
        const val ROTATION: Byte = 0x07
    }
}