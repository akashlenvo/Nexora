package com.godoy.nexora.video

import android.content.Context
import android.hardware.camera2.CaptureRequest
import android.util.Log
import android.util.Range
import android.util.Size
import androidx.annotation.OptIn
import androidx.camera.camera2.interop.Camera2Interop
import androidx.camera.camera2.interop.ExperimentalCamera2Interop
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.ImageAnalysis.Analyzer
import androidx.camera.core.ImageProxy
import androidx.camera.core.Preview
import androidx.camera.core.Preview.SurfaceProvider
import androidx.camera.core.resolutionselector.ResolutionSelector
import androidx.camera.core.resolutionselector.ResolutionStrategy
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.content.ContextCompat
import androidx.lifecycle.LifecycleOwner
import com.godoy.nexora.util.Logger
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

/**
 * The ImageReadyListener is responsible for closing the ImageProxy
 */
typealias ImageReadyListener = (image: ImageProxy) -> Unit

class Camera(
    private val surface: Preview.SurfaceProvider,
    private val format: Int,
    private val imageReadyListener: ImageReadyListener,
    private val context: Context,
    private val lifecycleOwner: LifecycleOwner
) {

    private val TAG = "Nexora"

    private val cameraExecutor: ExecutorService = Executors.newSingleThreadExecutor()
    private lateinit var resolution: Size

    private fun buildResolutionSelector(resolution: Size) =
        ResolutionSelector.Builder()
            .setResolutionStrategy(ResolutionStrategy(resolution, ResolutionStrategy.FALLBACK_RULE_NONE))
            .build()

    @OptIn(ExperimentalCamera2Interop::class)
    private fun buildPreview(resolutionSelector: ResolutionSelector, surface: SurfaceProvider) =
        Preview.Builder()
            .setResolutionSelector(resolutionSelector)
            .build()
            .apply {
                surfaceProvider = surface
            }

    @OptIn(ExperimentalCamera2Interop::class)
    private fun buildAnalyzer(resolutionSelector: ResolutionSelector, analyzer: Analyzer) =
        ImageAnalysis.Builder()
            .setResolutionSelector(resolutionSelector)
            .setOutputImageFormat(format)
            .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
            .build()
            .apply {
                setAnalyzer(cameraExecutor, analyzer)
            }

    @OptIn(ExperimentalCamera2Interop::class)
    fun start(resolution: Size, cameraSelector: CameraSelector) {
        this.resolution = resolution

        val cameraProviderFuture = ProcessCameraProvider.getInstance(context)

        cameraProviderFuture.addListener({

            val resolutionSelector = buildResolutionSelector(resolution)
            val cameraProvider = cameraProviderFuture.get()

            val preview = buildPreview(resolutionSelector, surface)
            val imageAnalyzer = buildAnalyzer(resolutionSelector) { image ->
                imageReadyListener(image)
            }

            try {
                cameraProvider.unbindAll()
                cameraProvider.bindToLifecycle(lifecycleOwner, cameraSelector, preview, imageAnalyzer)
            } catch (e: Exception) {
                Logger.log("CAMERA", "Use case binding failed " + e.message)
            }

        }, ContextCompat.getMainExecutor(context));
    }

}
