package com.darusc.vcamdroid.video

import android.content.Context
import android.media.MediaCodec
import android.util.Size
import androidx.annotation.OptIn
import androidx.camera.camera2.interop.Camera2CameraInfo
import androidx.camera.camera2.interop.ExperimentalCamera2Interop
import androidx.camera.core.CameraSelector
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.content.ContextCompat

fun getSupportedResolutions(
    context: Context,
    onResult: (back: List<Pair<Int, Int>>, front: List<Pair<Int, Int>>) -> Unit
) {
    val cameraProviderFuture = ProcessCameraProvider.getInstance(context)

    cameraProviderFuture.addListener({
        try {
            val cameraProvider = cameraProviderFuture.get()
            val backResolutions = getResolutionsForSelector(
                cameraProvider,
                CameraSelector.DEFAULT_BACK_CAMERA
            )
            val frontResolutions = getResolutionsForSelector(
                cameraProvider,
                CameraSelector.DEFAULT_FRONT_CAMERA
            )

            onResult(backResolutions, frontResolutions)
        } catch (e: Exception) {
            e.printStackTrace()
            onResult(emptyList(), emptyList())
        }
    }, ContextCompat.getMainExecutor(context))
}

@OptIn(ExperimentalCamera2Interop::class)
private fun getResolutionsForSelector(
    provider: ProcessCameraProvider,
    selector: CameraSelector
): List<Pair<Int, Int>> {

    try {
        if (!provider.hasCamera(selector)) return emptyList()
    } catch (e: Exception) {
        return emptyList()
    }

    val cameraInfo = provider.availableCameraInfos.firstOrNull {
        selector.filter(listOf(it)).isNotEmpty()
    } ?: return emptyList()

    val map = Camera2CameraInfo.from(cameraInfo).getCameraCharacteristic(
        android.hardware.camera2.CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP
    ) ?: return emptyList()

    val videoSizes: Array<Size> = map.getOutputSizes(MediaCodec::class.java) ?: emptyArray()

    return videoSizes
        .filter { size ->
            val ratio = size.width.toFloat() / size.height.toFloat()
            val is16by9 = ratio > 1.70 && ratio < 1.85
            val is4by3 = ratio > 1.30 && ratio < 1.36
            is16by9 || is4by3
        }
        .sortedByDescending { it.width }
        .map { Pair(it.width, it.height) }
}