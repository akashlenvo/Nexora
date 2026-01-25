package com.darusc.vcamdroid

import android.content.Intent
import android.os.Bundle
import android.view.SurfaceHolder
import android.view.WindowManager
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.darusc.vcamdroid.databinding.ActivityStreamBinding
import com.darusc.vcamdroid.networking.ConnectionManager
import com.darusc.vcamdroid.networking.PacketType
import com.darusc.vcamdroid.rtsp.Streamer
import com.darusc.vcamdroid.rtsp.StreamOptions
import com.darusc.vcamdroid.util.Logger

class StreamActivity : AppCompatActivity(), SurfaceHolder.Callback, ConnectionManager.ConnectionStateCallback {

    private val TAG = "VCamdroid"

    private lateinit var viewBinding: ActivityStreamBinding

    private val connectionManager = ConnectionManager.getInstance(this)
    private lateinit var streamer: Streamer

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        viewBinding = ActivityStreamBinding.inflate(layoutInflater)
        viewBinding.surfaceView.holder.addCallback(this)

        setContentView(viewBinding.root)

        // Disable navigating to logs activity for now as it breaks streaming
//        viewBinding.logReportButton.setOnClickListener {
//            val intent = Intent(this, LogActivity::class.java)
//            startActivity(intent)
//        }

        connectionManager.setOnBytesReceivedCallback(::onBytesReceived)
        streamer = Streamer(StreamOptions(), this, viewBinding.surfaceView)
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        if (holder.surface != null && holder.surface.isValid) {
            streamer.startPreview()
        }
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) { }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        streamer.stop()
    }

    override fun onDisconnected() {
        Toast.makeText(this, "Connection closed by server", Toast.LENGTH_LONG).show()
        Logger.log("STREAM", "TCP server disconnected")
        streamer.stop()
        finish()
    }

    private fun onBytesReceived(buffer: ByteArray, bytes: Int) {
        println(buffer)
        // Packet type is always the first byte
        val type = buffer[0]

        when (type) {
            PacketType.ACTIVATION -> {
                val options = StreamOptions.deserialize(buffer)
                streamer.startStream(options)
            }
            PacketType.CAMERA -> {
                streamer.switchCamera()
            }
            PacketType.RESOLUTION -> {
                val width = (buffer[1].toInt() and 0xFF) or (buffer[2].toInt() shl 8)
                val height = (buffer[3].toInt() and 0xFF) or (buffer[4].toInt() shl 8)
                streamer.setResolution(width, height)
            }
            PacketType.ROTATION -> {
                val degrees = buffer[1].toInt();
                streamer.rotate(degrees)
            }
            PacketType.EFFECT_FILTER -> {
                val filterName = String(buffer, 2, buffer[1].toInt(), Charsets.UTF_8)
                streamer.applyEffectFilter(filterName)
            }
            PacketType.CORRECTION_FILTER -> {
                val filterName = String(buffer, 2, buffer[1].toInt(), Charsets.UTF_8)
                val value = buffer[2 + buffer[1].toInt()].toInt()
                streamer.applyCorrectionFilter(filterName, value)
            }
            PacketType.BITRATE -> {
                val bitrate = (buffer[1].toInt() and 0xFF) or (buffer[2].toInt() shl 8)
                streamer.setBitrate(bitrate)
            }
            PacketType.ADAPTIVE_BITRATE ->  {
                val min = (buffer[1].toInt() and 0xFF) or (buffer[2].toInt() shl 8)
                val max = (buffer[3].toInt() and 0xFF) or (buffer[4].toInt() shl 8)
                streamer.setAdaptiveBitrate(min, max)
            }
            PacketType.STABILIZATION -> {
                streamer.setStabilization(buffer[1].toInt() == 1)
            }
            PacketType.FLASH -> {
                streamer.setFlash(buffer[1].toInt() == 1)
            }
            PacketType.FOCUS -> {
                streamer.setFocus(buffer[1].toInt())
            }
            PacketType.CODEC -> {
                streamer.setH265Codec(buffer[1].toInt() == 1)
            }
            PacketType.FPS -> {
                streamer.setFps(buffer[1].toInt())
            }
            PacketType.ZOOM -> {
                val factor = (buffer[1].toInt() and 0xFF) or
                        ((buffer[2].toInt() and 0xFF) shl 8) or
                        ((buffer[3].toInt() and 0xFF) shl 16) or
                        ((buffer[4].toInt() and 0xFF) shl 24)
                streamer.setZoom(Float.fromBits(factor))
            }
            PacketType.FLIP -> {
                val axis = buffer[1].toInt()
                streamer.flip(if (axis == 0) StreamOptions.FlipAxis.VERTICAL else StreamOptions.FlipAxis.HORIZONTAL)
            }
        }
    }
}