package com.darusc.vcamdroid.rtsp

import android.content.Context
import android.os.Handler
import android.os.Looper
import android.util.Log
import com.darusc.vcamdroid.networking.ConnectionManager
import com.darusc.vcamdroid.networking.DeviceDescriptor
import com.darusc.vcamdroid.video.filters.FilterAdjuster
import com.darusc.vcamdroid.video.filters.FilterRepository
import com.darusc.vcamdroid.video.getSupportedResolutions
import com.pedro.common.ConnectChecker
import com.pedro.encoder.input.gl.render.filters.BaseFilterRender
import com.pedro.encoder.input.video.CameraOpenException
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

    // The active effect filter. Only one can be active at a time
    private var activeEffectFilter: BaseFilterRender? = null

    // All the active correction filters. Multiple can be active at a time
    private var activeCorrectionFilters: MutableSet<BaseFilterRender> = HashSet()

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

    fun start() {
        startPreview()
        startStream()
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
            activeEffectFilter?.let { rtspServerCamera2.glInterface.removeFilter(it) }
            // Only apply the new requested effect filter if it is different than the NONE filter
            // Applying the NONE filter would completely remove all filters, including the correction filters
            // This behaviour results in deleting the effect filter but keeping the corrections
            if (category != FilterRepository.Category.NONE) {
                val filter = FilterRepository.create(filterName)
                if (filter != null) {
                    rtspServerCamera2.glInterface.addFilter(filter)
                    activeEffectFilter = filter
                }
            } else {
                activeEffectFilter = null
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

        val existingFilter = activeCorrectionFilters.find { it::class.java == fclass }
        if(existingFilter != null) {
            // If the filter already exist just update its adjustment value
            FilterAdjuster.adjust(existingFilter, value)
        } else {
            // Otherwise create a new filter, save it and apply it to the opengl interface
            val filter = FilterRepository.create(filterName)
            CoroutineScope(Dispatchers.Main).launch {
                FilterAdjuster.adjust(filter!!, value)
                activeCorrectionFilters.add(filter)
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

    /**
     * Set a new bitrate
     */
    fun setBitrate(bitrate: Int) {
        if (options.bitrate == bitrate)
            return

        options.bitrate = bitrate
        try {
            rtspServerCamera2.setVideoBitrateOnFly(bitrate)
            Log.d(TAG, "Switched bitrate to $bitrate")
        } catch (e: Exception) {
            Log.d(TAG, "Error while switching bitrate", e)
        }
    }

    private fun startPreview() {
        rtspServerCamera2.startPreview(options.camera, options.width, options.height)
    }

    private fun startStream() {
        if (!rtspServerCamera2.isStreaming) {
            val prepared = try {
                rtspServerCamera2.prepareVideo(
                    options.width,
                    options.height,
                    options.fps,
                    options.bitrate,
                    0
                )
            } catch (e: Exception) {
                false
            }

            if (prepared) {
                rtspServerCamera2.startStream(URL)
                Log.d(TAG, "Stream started")
            } else {
                Log.d(TAG, "Error preparing stream")
            }
        }
    }

    private fun restartStream() {
        Handler(Looper.getMainLooper()).post {
            val wasStreaming = rtspServerCamera2.isStreaming

            rtspServerCamera2.stopStream()
            rtspServerCamera2.stopPreview()

            startPreview()
            if (wasStreaming) {
                startStream()
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

    override fun onAuthError() {}

    override fun onAuthSuccess() {}

    override fun onConnectionFailed(reason: String) {}

    override fun onConnectionStarted(url: String) {}

    override fun onConnectionSuccess() {}

    override fun onDisconnect() {}
}