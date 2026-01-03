package com.darusc.vcamdroid.video

import android.content.Context
import android.graphics.Rect
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

    private val TAG = "VCamdroid"

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
                Log.e("VCamdroid", "Use case binding failed", e)
            }

        }, ContextCompat.getMainExecutor(context));
    }

    /**
     * Transforms a rectangle from screen coordinates to image coordinates
     *
     * https://github.com/googlesamples/mlkit/issues/388
     *
     * https://medium.com/@saiful103a/scan-barcode-only-when-they-are-inside-a-specific-area-of-the-preview-android-fcf8e02404d7
     */
    fun screenRectToImageRect(
        screenRect: Rect,
        screenSize: Size,
    ): Rect {
        val imageAspectRatio = resolution.height.toFloat() / resolution.width
        val screenAspectRatio = screenSize.width.toFloat() / screenSize.height

        var scaleX = 1f
        var scaleY = 1f
        var dx = 0f
        var dy = 0f

        if (imageAspectRatio > screenAspectRatio) {
            scaleY = screenSize.height.toFloat() / resolution.width
            scaleX = scaleY
            dx = (screenSize.width - resolution.height * scaleX) / 2
        } else {
            scaleX = screenSize.width.toFloat() / resolution.height
            scaleY = scaleX
            dy = (screenSize.height - resolution.width * scaleY) / 2
        }

        // Transform the screen space coordinates rectangle to image space coordinates
        val left = ((screenRect.left - dx) / scaleX).toInt()
        val top = ((screenRect.top - dy) / scaleY).toInt()
        val right = ((screenRect.right - dx) / scaleX).toInt()
        val bottom = ((screenRect.bottom - dy) / scaleY).toInt()

        return Rect(left, top, right, bottom)
    }
}