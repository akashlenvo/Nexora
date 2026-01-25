package com.darusc.vcamdroid

import android.app.Application
import android.content.Context
import android.content.Intent
import android.os.Process
import android.util.Log
import com.darusc.vcamdroid.util.Logger
import java.io.PrintWriter
import java.io.StringWriter
import kotlin.system.exitProcess

class VcApplication : Application() {

    private class CrashHandler(private val context: Context) : Thread.UncaughtExceptionHandler {

        override fun uncaughtException(thread: Thread, throwable: Throwable) {
            // Capture the Stack Trace
            val sw = StringWriter()
            val pw = PrintWriter(sw)
            throwable.printStackTrace(pw)
            val stackTrace = sw.toString()

            // Log it to console (just in case)
            Log.e("CrashHandler", "Fatal Crash: $stackTrace")

            try {
                // Prepare the Intent to restart into LogActivity
                val intent = Intent(context, LogActivity::class.java).apply {
                    // NEW_TASK: Required because we are starting from a non-activity context
                    // CLEAR_TASK: Wipes the corrupted activity stack so the user can't "back" into the crash
                    flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK

                    // Pass the crash data so it survives the process restart
                    putExtra("CRASH_REPORT", stackTrace)
                }

                // Launch the Log Screen
                context.startActivity(intent)

            } catch (e: Exception) {
                Log.e("CrashHandler", "Failed to switch to LogActivity", e)
            } finally {
                // Kill the broken process explicitly
                // If we don't do this, the app might hang or enter an undefined state
                Process.killProcess(Process.myPid())
                exitProcess(1)
            }
        }
    }

    override fun onCreate() {
        super.onCreate()
        Logger.init(this)
        Thread.setDefaultUncaughtExceptionHandler(CrashHandler(this))
    }
}