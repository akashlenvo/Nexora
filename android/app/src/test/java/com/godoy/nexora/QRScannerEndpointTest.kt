package com.godoy.nexora

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Test

class QRScannerEndpointTest {
    @Test
    fun acceptsLocalIpv4Endpoint() {
        val endpoint = QRScanner.parseEndpoint(" 192.168.1.5:6969 ")
        assertEquals("192.168.1.5", endpoint?.address)
        assertEquals(6969, endpoint?.port)
    }

    @Test
    fun rejectsMalformedAddressesAndPorts() {
        listOf(
            "192.168.1.5", "192.168.1.5:0", "192.168.1.5:65536",
            "192.168.1.5:abc", "999.168.1.5:6969", "192.168.1.5:6969:1"
        ).forEach { assertNull(QRScanner.parseEndpoint(it)) }
    }
}
