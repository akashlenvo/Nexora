package com.darusc.vcamdroid

import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.pm.PackageManager
import android.graphics.Rect
import android.hardware.camera2.CaptureRequest
import android.net.ConnectivityManager
import android.net.NetworkCapabilities
import android.net.Uri
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.provider.Settings
import android.util.Log
import android.util.Size
import android.view.View
import android.view.WindowManager
import android.widget.Toast
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AlertDialog
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.ImageProxy
import androidx.camera.view.PreviewView
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.darusc.vcamdroid.databinding.ActivityMainBinding
import com.darusc.vcamdroid.networking.ConnectionManager
import com.darusc.vcamdroid.video.Camera
import java.security.Permission

class MainActivity : AppCompatActivity(), ConnectionManager.ConnectionStateCallback {

    private lateinit var viewBinding: ActivityMainBinding

    private val qrscanner = QRScanner()
    private var connectionManager = ConnectionManager.getInstance(this)
    private var camera: Camera? = null

    private var isConnecting = false

    private val TAG = "VCamdroid"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        viewBinding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(viewBinding.root)

        enableEdgeToEdge()
        initialize()
    }

    override fun onResume() {
        super.onResume()
        connectionManager = ConnectionManager.getInstance(this)
        if(camera != null) {
            camera!!.start(Size(1280, 720), CameraSelector.DEFAULT_BACK_CAMERA, CaptureRequest.CONTROL_AWB_MODE_AUTO, CaptureRequest.CONTROL_EFFECT_MODE_OFF)
            connectWIFI()
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if(requestCode == 1000) {
            if(grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                initialize()
            } else {
                // If camera permission was not granted show a prompt to the user
                // to go to settings and enable the required permission

                AlertDialog.Builder(this)
                    .setTitle("Camera Permission Required")
                    .setMessage("Please enable camera permission in settings and restart the app.")
                    .setPositiveButton("Go to settings") { _, _ ->
                        val intent = Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS).apply {
                            data = Uri.fromParts("package", packageName, null)
                        }
                        startActivity(intent)
                    }
                    .setNegativeButton("Cancel", null)
                    .show()
            }
        }
    }

    override fun onConnectionSuccessful(connectionMode: ConnectionManager.Mode) {
        qrscanner.stop()
        Log.d(TAG, "Connection successful $connectionMode")

        val intent = Intent(this, StreamActivity::class.java)
        startActivity(intent)
    }

    override fun onConnectionFailed(connectionMode: ConnectionManager.Mode) {
        if (connectionMode == ConnectionManager.Mode.USB && hasWifiConnection()) {
            // If usb connection failed try again over wifi
            connectWIFI()
        } else {
            isConnecting = false
            qrscanner.stop()
            Log.e(TAG, "Error: Cannot connect!")
        }
    }

    /**
     * Check for CAMERA permission and request it if not granted
     */
    private fun checkPermissions(): Boolean {
        if(ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.CAMERA), 1000)
            return false
        }

        return true
    }

    /**
     * If the required permissions are grante create the camera instance,
     * start it, and initialize the connection procedure
     */
    private fun initialize() {
        if(checkPermissions()) {
            camera = Camera(
                viewBinding.viewFinder.surfaceProvider,
                ImageAnalysis.OUTPUT_IMAGE_FORMAT_YUV_420_888,
                ::processImage,
                this,
                this
            )
            camera!!.start(Size(1280, 720), CameraSelector.DEFAULT_BACK_CAMERA, CaptureRequest.CONTROL_AWB_MODE_AUTO, CaptureRequest.CONTROL_EFFECT_MODE_OFF)

            // Prioritize the usb connection through adb
            if (hasUsbConnection()) {
                connectUSB()
            } else if (hasWifiConnection()) {
                connectWIFI()
            }
        }
    }

    private fun processImage(imageProxy: ImageProxy) {
        // Process each incoming image from the camera
        // launchScanTask() will scan and call the callback on success
        // only if start() was called before
        qrscanner.launchScanTask(imageProxy, camera?.screenRectToImageRect(viewBinding.overlay.rect, viewBinding.overlay.size) ?: Rect()) { result ->
            if(result != null) {
                connectionManager.connect(result.address, result.port)
            } else {
                Log.d(TAG, "Invalid QR code")
            }
        }
    }

    private fun hasUsbConnection(): Boolean {
        val intent = applicationContext.registerReceiver(
            null,
            IntentFilter("android.hardware.usb.action.USB_STATE")
        )
        return intent?.getBooleanExtra("connected", false) == true
    }

    private fun hasWifiConnection(): Boolean {
        val connectivityManager = applicationContext.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        val network = connectivityManager.activeNetwork ?: return false
        val capabilities = connectivityManager.getNetworkCapabilities(network) ?: return false
        return capabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI)
    }

    private fun connectUSB() {
        connectionManager.connect(6969)
    }

    private fun connectWIFI() {
        // Start the QRScanner so it can scan the image frames received from the camera
        // Actual WIFI connection is tried only on scan success
        qrscanner.start()
    }
}