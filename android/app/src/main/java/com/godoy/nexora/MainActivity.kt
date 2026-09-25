package com.godoy.nexora

import android.Manifest
import android.content.Intent
import android.content.IntentFilter
import android.content.pm.PackageManager
import android.net.Uri
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.provider.Settings
import android.util.Size
import android.widget.EditText
import android.widget.Toast
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AlertDialog
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.ImageProxy
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.godoy.nexora.databinding.ActivityMainBinding
import com.godoy.nexora.networking.ConnectionManager
import com.godoy.nexora.util.Logger
import com.godoy.nexora.video.Camera
import com.google.android.material.dialog.MaterialAlertDialogBuilder

class MainActivity : AppCompatActivity(), ConnectionManager.ConnectionStateCallback {

    private lateinit var viewBinding: ActivityMainBinding

    private val qrscanner = QRScanner()
    private var connectionManager = ConnectionManager.getInstance(this)
    private var camera: Camera? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        viewBinding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(viewBinding.root)

        viewBinding.logReportButton.setOnClickListener {
            val intent = Intent(this, LogActivity::class.java)
            startActivity(intent)
        }
        viewBinding.manualConnectButton.setOnClickListener { showManualConnectDialog() }

        enableEdgeToEdge()
        initialize()
    }

    override fun onResume() {
        super.onResume()
        connectionManager = ConnectionManager.getInstance(this)
        if(camera != null) {
            camera!!.start(Size(1280, 720), CameraSelector.DEFAULT_BACK_CAMERA)
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

                MaterialAlertDialogBuilder(this)
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
        runOnUiThread {
            qrscanner.stop()
            Logger.log("MAIN", "Connection successful $connectionMode")
            startActivity(Intent(this, StreamActivity::class.java))
        }
    }

    override fun onConnectionFailed(connectionMode: ConnectionManager.Mode) {
        runOnUiThread {
            if (connectionMode == ConnectionManager.Mode.USB) {
                // If usb connection failed try again over wifi
                connectWIFI()
                Toast.makeText(this, "USB connection failed. Scan the QR or enter the computer IP.", Toast.LENGTH_LONG).show()
            } else {
                qrscanner.start()
                Logger.log("MAIN", "Error: Cannot connect!")
                Toast.makeText(this, "Connection failed. Check the computer IP and port, then try again.", Toast.LENGTH_LONG).show()
            }
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
            camera!!.start(Size(1280, 720), CameraSelector.DEFAULT_BACK_CAMERA)

            // Prioritize the usb connection through adb
            if (hasUsbConnection()) {
                connectUSB()
            } else {
                connectWIFI()
            }
        }
    }

    private fun processImage(imageProxy: ImageProxy) {
        // Process each incoming image from the camera
        // launchScanTask() will scan and call the callback on success
        // only if start() was called before
        qrscanner.launchScanTask(imageProxy) { result ->
            runOnUiThread {
                qrscanner.stop()
                MaterialAlertDialogBuilder(this)
                    .setTitle("Connect via WiFi")
                    .setMessage("Connect to ${result.address}:${result.port}?")
                    .setPositiveButton("Connect") { _, _ ->
                        connectionManager.connect(result.address, result.port)
                    }
                    .setNegativeButton("Cancel") { _, _ -> qrscanner.start() }
                    .show()
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

    private fun showManualConnectDialog() {
        qrscanner.stop()
        val input = EditText(this).apply {
            hint = getString(R.string.ip_address_hint)
            setSingleLine(true)
        }
        val dialog = MaterialAlertDialogBuilder(this)
            .setTitle(R.string.connect_by_ip)
            .setView(input)
            .setPositiveButton("Connect", null)
            .setNegativeButton("Cancel") { _, _ -> qrscanner.start() }
            .create()
        dialog.setOnShowListener {
            dialog.getButton(AlertDialog.BUTTON_POSITIVE).setOnClickListener {
                val endpoint = QRScanner.parseEndpoint(input.text.toString())
                if (endpoint == null) {
                    input.error = "Enter a valid IPv4 address and port (1–65535)"
                } else {
                    dialog.dismiss()
                    connectionManager.connect(endpoint.address, endpoint.port)
                }
            }
        }
        dialog.setOnCancelListener { qrscanner.start() }
        dialog.show()
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
