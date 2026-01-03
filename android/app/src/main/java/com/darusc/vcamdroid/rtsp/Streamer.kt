package com.darusc.vcamdroid.rtsp

import android.content.Context
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.Display
import android.view.ViewOverlay
import com.darusc.vcamdroid.networking.ConnectionManager
import com.darusc.vcamdroid.networking.DeviceDescriptor
import com.darusc.vcamdroid.video.filters.FilterAdjuster
import com.darusc.vcamdroid.video.filters.FilterRepository
import com.darusc.vcamdroid.video.filters.custom.HorizontalFlipFilterRender
import com.darusc.vcamdroid.video.filters.custom.VerticalFlipFilterRender
import com.darusc.vcamdroid.video.getSupportedResolutions
import com.pedro.common.ConnectChecker
import com.pedro.common.VideoCodec
import com.pedro.encoder.input.gl.render.filters.BaseFilterRender
import com.pedro.encoder.input.video.CameraOpenException
import com.pedro.library.util.BitrateAdapter
import com.pedro.library.view.OpenGlView
import com.pedro.rtspserver.RtspServerCamera2
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.util.logging.Filter

class Streamer(
    private var options: StreamOptions,
    context: Context,
    openGlView: OpenGlView,
) : ConnectChecker {

    companion object {
        const val PORT = 8554
        const val URL = "rtsp://localhost:$PORT/live"
    }

    private val TAG = "VCamdroid"

    private val rtspServerCamera2 = RtspServerCamera2(openGlView, this, PORT)

    private var bitrateAdapter = BitrateAdapter {
        if (options.adaptiveBitrateEnabled) {
            rtspServerCamera2.setVideoBitrateOnFly(it)
        }
    }

    init {
        val connectionManager = ConnectionManager.getInstance()
        val localIpAddress = connectionManager.localIpAddress

        getSupportedResolutions(context) { back, front ->
            val descriptor = DeviceDescriptor(
                android.os.Build.MODEL,
                URL.replace("localhost", localIpAddress),
                back,
                front,
                FilterRepository.filters
            )

            connectionManager.sendDescriptor(descriptor)
        }
    }

    fun stop() {
        rtspServerCamera2.stopPreview()
        if (rtspServerCamera2.isStreaming) {
            rtspServerCamera2.stopStream()
        }
    }

    /**
     * Switch camera front <-> back
     */
    fun switchCamera() {
        try {
            rtspServerCamera2.switchCamera()
            options.camera = rtspServerCamera2.cameraFacing
        } catch (e: CameraOpenException) {
            Log.d(
                TAG,
                "Can't switch camera. Current resolution ({${options.width}, ${options.height}) not supported"
            )
        }
    }

    /**
     * Set a new resolution. Requires a stream restart
     */
    fun setResolution(width: Int, height: Int) {
        if (options.width == width && options.height == height)
            return

        Log.d(TAG, "Changing resolution to ${width}x${height}...")
        options.width = width
        options.height = height
        options.bitrate = calculateOptimalBitrate(width, height, options.fps)
        restartStream()
    }

    fun rotate(degress: Int) {
        options.rotation += degress
        rtspServerCamera2.glInterface.setStreamRotation(options.rotation)
        // rtspServerCamera2.glInterface.setRotation(options.rotation)
    }

    /**
     * Apply an effect filter. Only one active at a time
     * @param filterName Filter's name
     */
    fun applyEffectFilter(filterName: String) {
        val category = FilterRepository.getCategory(filterName)
        if (category == FilterRepository.Category.CORRECTION) {
            return
        }

        CoroutineScope(Dispatchers.Main).launch {
            // Remove the current filter if there is one
            options.activeEffectFilter?.let { rtspServerCamera2.glInterface.removeFilter(it) }
            // Only apply the new requested effect filter if it is different than the NONE filter
            // Applying the NONE filter would completely remove all filters, including the correction filters
            // This behaviour results in deleting the effect filter but keeping the corrections
            if (category != FilterRepository.Category.NONE) {
                val filter = FilterRepository.create(filterName)
                if (filter != null) {
                    rtspServerCamera2.glInterface.addFilter(filter)
                    options.activeEffectFilter = filter
                }
            } else {
                options.activeEffectFilter = null
            }
        }
    }

    /**
     * Apply a correction filter. Multiple filters can be active at once.
     * If the filter was already applied it is adjusted with new value
     */
    fun applyCorrectionFilter(filterName: String, value: Int) {
        val category = FilterRepository.getCategory(filterName)
        val fclass = FilterRepository.getClass(filterName)

        if (category != FilterRepository.Category.CORRECTION || fclass == null) {
            return
        }

        val existingFilter = options.activeCorrectionFilters.find { it::class.java == fclass }
        if (existingFilter != null) {
            // If the filter already exist just update its adjustment value
            FilterAdjuster.adjust(existingFilter, value)
        } else {
            // Otherwise create a new filter, save it and apply it to the opengl interface
            val filter = FilterRepository.create(filterName)
            CoroutineScope(Dispatchers.Main).launch {
                FilterAdjuster.adjust(filter!!, value)
                options.activeCorrectionFilters.add(filter)
                rtspServerCamera2.glInterface.addFilter(filter)
            }
        }
    }

    /**
     * Set a new target fps. Requires a stream restart
     */
    fun setFps(fps: Int) {
        if (options.fps == fps)
            return

        Log.d(TAG, "Changing fps to ${fps}...")
        options.fps = fps
        restartStream()
    }

    fun setZoom(zoom: Float) {
        rtspServerCamera2.zoom = zoom
    }

    fun flip(axis: StreamOptions.FlipAxis) {
        CoroutineScope(Dispatchers.Main).launch {
            when (axis) {
                StreamOptions.FlipAxis.HORIZONTAL -> options.hFlipFilter = toggleFilter(options.hFlipFilter) { HorizontalFlipFilterRender() } as HorizontalFlipFilterRender?
                StreamOptions.FlipAxis.VERTICAL -> options.vFlipFilter = toggleFilter(options.vFlipFilter) { VerticalFlipFilterRender() } as VerticalFlipFilterRender?
            }
        }
    }

    private fun toggleFilter(
        currentFilter: BaseFilterRender?,
        createFilter: () -> BaseFilterRender
    ): BaseFilterRender? {
        return if (currentFilter != null) {
            // If it exists, remove it and return null (to clear the variable)
            rtspServerCamera2.glInterface.removeFilter(currentFilter)
            null
        } else {
            // If it's null, create new one, add it, and return it
            val newFilter = createFilter()
            rtspServerCamera2.glInterface.addFilter(newFilter)
            newFilter
        }
    }

    /**
     * Set a new bitrate
     */
    fun setBitrate(bitrate: Int) {
        options.adaptiveBitrateEnabled = false
        rtspServerCamera2.setVideoBitrateOnFly(bitrate * 1024)
    }

    fun setAdaptiveBitrate(min: Int, max: Int) {
        options.adaptiveBitrateEnabled = true
        options.adaptiveBitrateMin = min * 1024
        options.adaptiveBitrateMax = max * 1024

        bitrateAdapter.setMaxBitrate(options.adaptiveBitrateMax)
        rtspServerCamera2.setVideoBitrateOnFly(options.adaptiveBitrateMax)
    }

    fun setStabilization(enabled: Boolean) {
        when (enabled) {
            true -> rtspServerCamera2.enableVideoStabilization()
            false -> rtspServerCamera2.disableVideoStabilization()
        }
    }

    fun setFlash(enabled: Boolean) {
        when (enabled) {
            true -> rtspServerCamera2.enableLantern()
            false -> rtspServerCamera2.disableLantern()
        }
    }

    fun setFocus(mode: Int) {
        when (mode) {
            0 -> rtspServerCamera2.enableAutoFocus()
            1 -> rtspServerCamera2.disableAutoFocus()
        }
    }

    fun setH265Codec(enabled: Boolean) {
        val codec = if (enabled) VideoCodec.H265 else VideoCodec.H264
        rtspServerCamera2.setVideoCodec(codec)

        // Only restart if we are actually streaming
        if (rtspServerCamera2.isStreaming) {
            restartStream()
        }
    }

    fun startPreview() {
        if (!rtspServerCamera2.isOnPreview) {
            rtspServerCamera2.startPreview(options.camera, options.width, options.height)
        }
    }

    fun startStream(newOptions: StreamOptions) {
        val oldOptions = this.options
        this.options = newOptions

        Handler(Looper.getMainLooper()).post {
            val needPreviewRestart = (oldOptions.width != newOptions.width) ||
                    (oldOptions.height != newOptions.height) ||
                    (oldOptions.camera != newOptions.camera)

            if (needPreviewRestart && rtspServerCamera2.isOnPreview) {
                rtspServerCamera2.stopPreview()
            }

            if (!rtspServerCamera2.isOnPreview) {
                rtspServerCamera2.startPreview()
            }

            applyOptionsToStream(newOptions)
            startStreamInternal()
        }
    }

    private fun startStreamInternal() {
        if (!rtspServerCamera2.isStreaming) {
            try {
                rtspServerCamera2.prepareVideo(
                    options.width,
                    options.height,
                    options.fps,
                    options.bitrate,
                    0
                )
                rtspServerCamera2.startStream(URL)
                Log.d(TAG, "Stream started")
            } catch (e: Exception) {
                Log.d(TAG, "Error preparing stream")
            }
        }
    }

    /**
     * Apply initial options
     */
    private fun applyOptionsToStream(options: StreamOptions) {
        if (options.adaptiveBitrateEnabled) {
            bitrateAdapter.setMaxBitrate(options.adaptiveBitrateMax)
        } else {
            rtspServerCamera2.setVideoBitrateOnFly(options.bitrate)
        }

        setStabilization(options.stabilization)
        setFocus(options.focusMode)
        rtspServerCamera2.setVideoCodec(if (options.h265Enabled) VideoCodec.H265 else VideoCodec.H264)

        options.activeEffectFilter?.let { rtspServerCamera2.glInterface.addFilter(it) }
        options.activeCorrectionFilters.forEach {
            rtspServerCamera2.glInterface.addFilter(it)
        }
    }

    private fun restartStream() {
        Handler(Looper.getMainLooper()).post {
            val wasStreaming = rtspServerCamera2.isStreaming

            rtspServerCamera2.stopStream()
            rtspServerCamera2.stopPreview()

            startPreview()
            if (wasStreaming) {
                startStreamInternal()
            }
        }
    }

    /**
     * Calculate the optimal bitrate for given resolution and fps.
     * Uses 1080p 30fps @ 4000Mbps as a standard reference
     */
    private fun calculateOptimalBitrate(width: Int, height: Int, fps: Int): Int {
        val targetPixelCount = width.toLong() * height.toLong() * fps
        val referencePixelCount = 1920L * 1080L * 30L
        val referenceBitrate = 6000 * 1024L

        val calculatedBitrate =
            (targetPixelCount.toDouble() / referencePixelCount.toDouble()) * referenceBitrate
        val minBitrate = 500 * 1024
        val maxBitrate = 12000 * 1024

        return calculatedBitrate.toInt().coerceIn(minBitrate, maxBitrate)
    }

    override fun onNewBitrate(bitrate: Long) {
        bitrateAdapter.adaptBitrate(bitrate)
    }

    override fun onAuthError() {}

    override fun onAuthSuccess() {}

    override fun onConnectionFailed(reason: String) {}

    override fun onConnectionStarted(url: String) {}

    override fun onConnectionSuccess() {}

    override fun onDisconnect() {}
}