package com.darusc.vcamdroid

import android.os.Bundle
import android.util.Log
import android.view.SurfaceHolder
import android.view.WindowManager
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.darusc.vcamdroid.databinding.ActivityStreamBinding
import com.pedro.common.ConnectChecker
import com.pedro.encoder.input.video.CameraHelper
import com.pedro.rtspserver.RtspServerCamera2

class StreamActivity : AppCompatActivity(), ConnectChecker, SurfaceHolder.Callback {

    private val TAG = "VCamdroid"
    private lateinit var viewBinding: ActivityStreamBinding
    private lateinit var rtspServer: RtspServerCamera2

    // Configuration
    private val vBitrate = 12000 * 1024 // 12 Mbps for 4K
    private val aBitrate = 128 * 1024
    private val width = 3840
    private val height = 2160
    private val fps = 30

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        viewBinding = ActivityStreamBinding.inflate(layoutInflater)
        setContentView(viewBinding.root)

        rtspServer = RtspServerCamera2(viewBinding.surfaceView, this, 8554)

        viewBinding.surfaceView.holder.addCallback(this)

//        val resolutions = rtspServer.resolutionsBack
//        for (size in resolutions) {
//            Log.d(TAG, "Supported Size: ${size.width} x ${size.height}")
//        }
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        if (holder.surface != null && holder.surface.isValid) {
            rtspServer.startPreview(CameraHelper.Facing.BACK, width, height)
            startStream()
        }
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        // RtspServerCamera2 handles orientation automatically
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        if (rtspServer.isStreaming) rtspServer.stopStream()
        rtspServer.stopPreview()
    }

    private fun startStream() {
        if (!rtspServer.isStreaming) {
            val prepared = try {
                rtspServer.prepareVideo(width, height, fps, vBitrate, 0)
            } catch (e: Exception) {
                false
            }

            if (prepared) {
                // rtspServer.prepareAudio(aBitrate, 44100, true, true, true)
                rtspServer.startStream("rtsp://localhost:8554/live")
                Log.d(TAG, "Server started")
            } else {
                Toast.makeText(this, "Error preparing stream", Toast.LENGTH_SHORT).show()
            }
        }
    }

    override fun onConnectionSuccess() {
        runOnUiThread { Toast.makeText(this, "Client Connected", Toast.LENGTH_SHORT).show() }
        Log.d(TAG, "Connection success")
    }

    override fun onConnectionFailed(reason: String) {
        runOnUiThread { Toast.makeText(this, "Connection failed: $reason", Toast.LENGTH_SHORT).show() }
        rtspServer.stopStream()
    }

    override fun onConnectionStarted(url: String) {
        Log.d(TAG, "Connection $url started")
    }

    override fun onNewBitrate(bitrate: Long) {
        Log.d(TAG, "New bitrate: $bitrate")
    }

    override fun onDisconnect() {
        runOnUiThread { Toast.makeText(this, "Client Disconnected", Toast.LENGTH_SHORT).show() }
        Log.d(TAG, "Disconnected")
    }

    override fun onAuthError() {
        runOnUiThread { Toast.makeText(this, "Auth Error", Toast.LENGTH_SHORT).show() }
        rtspServer.stopStream()
    }

    override fun onAuthSuccess() {
        Log.d(TAG, "Auth Success")
    }
}