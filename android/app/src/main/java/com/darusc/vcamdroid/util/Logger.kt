package com.darusc.vcamdroid.util

import android.content.Context
import android.os.Handler
import android.os.Looper
import android.util.Log
import java.io.File
import java.io.FileWriter
import java.io.IOException
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.util.concurrent.Executors

object Logger {

    // Internal Storage File (Private & Secure)
    private var logFile: File? = null

    // Background thread for file writing (prevents UI lag)
    private val fileExecutor = Executors.newSingleThreadExecutor()

    // In-Memory Buffer (For fast UI display)
    private val logBuffer = StringBuilder()
    private const val MAX_BUFFER_SIZE = 50000
    private const val PRUNE_SIZE = 10000

    // UI Listener
    var onLogAdded: ((String) -> Unit)? = null

    private val formatter = SimpleDateFormat("MM-dd HH:mm:ss.SSS", Locale.US)
    private val uiHandler = Handler(Looper.getMainLooper())

    /**
     * Call this in Application.onCreate()
     */
    fun init(context: Context) {
        // Path: /data/user/0/com.vcamdroid/files/app_logs.txt
        logFile = File(context.filesDir, "app_logs.txt")

        // Auto-Rotate: Delete file if it exceeds 5MB to save space
        if (logFile!!.exists() && logFile!!.length() > 5 * 1024 * 1024) {
            logFile!!.delete()
        }

        log("AppLogger", "=== Logger Initialized ===")
    }

    fun log(tag: String, message: String) {
        Log.d("Vcamdroid", "$tag: $message")

        val timestamp = formatter.format(Date())
        val formattedLog = "[$timestamp] $tag: $message\n"

        // Update Memory Buffer (Synchronized)
        synchronized(this) {
            logBuffer.append(formattedLog)
            if (logBuffer.length > MAX_BUFFER_SIZE) {
                logBuffer.delete(0, PRUNE_SIZE)
            }
        }

        // Notify UI (Main Thread)
        uiHandler.post {
            onLogAdded?.invoke(formattedLog)
        }

        // Write to File (Background Thread - Append Mode)
        fileExecutor.execute {
            try {
                logFile?.let { file ->
                    FileWriter(file, true).use { writer ->
                        writer.append(formattedLog)
                    }
                }
            } catch (e: IOException) {
                Log.e("AppLogger", "Failed to write to disk", e)
            }
        }
    }

    fun getLogs(): String {
        synchronized(this) {
            return logBuffer.toString()
        }
    }

    /**
     * Returns the raw file object for sharing
     */
    fun getLogFile(): File? = logFile
}