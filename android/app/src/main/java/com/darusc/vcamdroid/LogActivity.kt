package com.darusc.vcamdroid

import android.content.Intent
import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.FileProvider
import com.darusc.vcamdroid.databinding.ActivityLogsBinding
import com.darusc.vcamdroid.util.Logger

class LogActivity : AppCompatActivity() {

    private lateinit var binding: ActivityLogsBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityLogsBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Setup Share Button
        binding.btnSaveLogs.setOnClickListener {
            shareLogFile()
        }

        // Load Initial Logs
        // Check if opened from a Crash
        val crashReport = intent.getStringExtra("CRASH_REPORT")
        if (crashReport != null) {
            binding.logTextView.append(Logger.getLogs())
            binding.logTextView.append("\n=== CRASH DETECTED ===\n$crashReport\n\n")
        } else {
            binding.logTextView.text = Logger.getLogs()
        }

        scrollToBottom()
    }

    override fun onResume() {
        super.onResume()
        // Subscribe to real-time updates
        Logger.onLogAdded = { newLog ->
            binding.logTextView.append(newLog)
            scrollToBottom()
        }
    }

    override fun onPause() {
        super.onPause()
        // Unsubscribe to avoid leaks
        Logger.onLogAdded = null
    }

    private fun scrollToBottom() {
        binding.logScrollView.post {
            binding.logScrollView.fullScroll(android.view.View.FOCUS_DOWN)
        }
    }

    private fun shareLogFile() {
        val file = Logger.getLogFile()
        if (file == null || !file.exists()) {
            Toast.makeText(this, "Log file is empty or missing.", Toast.LENGTH_SHORT).show()
            return
        }

        try {
            // Securely share the private file using FileProvider
            val uri = FileProvider.getUriForFile(
                this,
                "${packageName}.fileprovider", // Must match Manifest
                file
            )

            val intent = Intent(Intent.ACTION_SEND).apply {
                type = "text/plain"
                putExtra(Intent.EXTRA_STREAM, uri)
                putExtra(Intent.EXTRA_SUBJECT, "VCamdroid Debug Logs")
                addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
            }

            startActivity(Intent.createChooser(intent, "Share Logs via..."))

        } catch (e: Exception) {
            Toast.makeText(this, "Share failed: ${e.message}", Toast.LENGTH_LONG).show()
            e.printStackTrace()
        }
    }
}