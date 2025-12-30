package com.darusc.vcamdroid.rtsp

import android.content.Context
import android.os.Handler
import android.os.Looper
import android.util.Log
import com.darusc.vcamdroid.networking.ConnectionManager
import com.darusc.vcamdroid.networking.DeviceDescriptor
import com.darusc.vcamdroid.video.getSupportedResolutions
import com.pedro.common.ConnectChecker
import com.pedro.encoder.input.video.CameraOpenException
import com.pedro.library.view.OpenGlView
import com.pedro.rtspserver.RtspServerCamera2

class Streamer(
    private var options: StreamOptions,
    context: Context,
    openGlView: OpenGlView,
) : ConnectChecker {

    companion object {
        const val PORT = 8554
        const val URL = "rtsp://localhost$PORT/live"
    }

    private val TAG = "VCamdroid"

    private val rtspServerCamera2 = RtspServerCamera2(openGlView, this, PORT)

    init {
        val connectionManager = ConnectionManager.getInstance()
        val localIpAddress = connectionManager.localIpAddress

        getSupportedResolutions(context) { back, front ->
            val descriptor = DeviceDescriptor(
                android.os.Build.MODEL,
                URL.replace("localhost", localIpAddress),
                back,
                front
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
        restartStream()
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

    override fun onAuthError() {}

    override fun onAuthSuccess() {}

    override fun onConnectionFailed(reason: String) {}

    override fun onConnectionStarted(url: String) {}

    override fun onConnectionSuccess() {}

    override fun onDisconnect() {}
}