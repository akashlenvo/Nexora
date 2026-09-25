package com.godoy.nexora

import androidx.annotation.OptIn
import androidx.camera.core.ExperimentalGetImage
import androidx.camera.core.ImageProxy
import com.google.mlkit.vision.barcode.BarcodeScanner
import com.google.mlkit.vision.barcode.BarcodeScannerOptions
import com.google.mlkit.vision.barcode.BarcodeScanning
import com.google.mlkit.vision.barcode.common.Barcode
import com.google.mlkit.vision.common.InputImage

class QRScanner() {

    data class Result(
        var address: String,
        var port: Int
    )

    private val options: BarcodeScannerOptions
    private val scanner: BarcodeScanner

    companion object {
        private val addressRegex = Regex("""((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)""")

        fun parseEndpoint(value: String): Result? {
            val parts = value.trim().split(":")
            if (parts.size != 2 || !parts[0].matches(addressRegex)) return null
            val port = parts[1].toIntOrNull() ?: return null
            if (port !in 1..65535) return null
            return Result(parts[0], port)
        }
    }

    @Volatile private var enabled = false

    init {
        options = BarcodeScannerOptions.Builder().setBarcodeFormats(Barcode.FORMAT_QR_CODE).build()
        scanner = BarcodeScanning.getClient(options)
    }

    fun start() {
        enabled = true
    }

    fun stop() {
        enabled = false
    }

    @OptIn(ExperimentalGetImage::class)
    fun launchScanTask(imageProxy: ImageProxy, callback: (Result) -> Unit) {
        if(!enabled) {
            imageProxy.close()
            return
        }

        val mediaImage = imageProxy.image
        if(mediaImage != null) {
            val image = InputImage.fromMediaImage(mediaImage, imageProxy.imageInfo.rotationDegrees)
            scanner.process(image)
                .addOnSuccessListener { barcodes ->
                    for (barcode in barcodes) {
                        if (!enabled) break
                        val endpoint = parseEndpoint(barcode.rawValue ?: "") ?: continue
                        // Stop the scanner. Further attempts at connecting should be
                        // manually triggered otherwise multiple connection might be established
                        // (for each frame of the camera)
                        stop()
                        callback(endpoint)
                        break
                    }
                    imageProxy.close()
                }
                .addOnFailureListener {
                    imageProxy.close()
                }
        } else {
            imageProxy.close()
        }
    }

}
