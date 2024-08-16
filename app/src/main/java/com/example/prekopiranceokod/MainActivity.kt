package com.example.prekopiranceokod

import android.os.Bundle
import android.os.Handler
import android.view.View
import android.widget.Button
import android.widget.CheckBox
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.snackbar.Snackbar

class MainActivity : AppCompatActivity() {

    private lateinit var handler: Handler
    private lateinit var textView: TextView
    private var isMonitoring = false
    private val updateInterval: Long = 1000 // 1 sekunda
    private val updateRunnable = object : Runnable {
        override fun run() {
            if (isMonitoring) {
                // Pozovite JNI funkciju koja vraća informacije
                val result = getResourceUsage()
                textView.text = result
                handler.postDelayed(this, updateInterval)
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        handler = Handler()
        textView = findViewById(R.id.text_view)

        // Inicialno postavljanje TextView na nevidljiv
        textView.visibility = View.GONE

        val buttonExisting = findViewById<Button>(R.id.button_existing)
        buttonExisting.setOnClickListener {
            // Pozovi JNI metodu za postojeće dugme
            val result = callNativeFunction()
            showSnackbar(result)
        }

        val buttonNew = findViewById<Button>(R.id.button_new)
        buttonNew.setOnClickListener {
            if (isMonitoring) {
                // Zaustavi monitoring
                stopMonitoring()
                showSnackbar("Monitoring stopped")
            } else {
                // Pokreni monitoring
                startMonitoring()
                showSnackbar("Monitoring started")
            }
            isMonitoring = !isMonitoring
        }
    }

    private fun showSnackbar(message: String) {
        val snackbarContainer: View = findViewById(R.id.snackbar_container)
        val snackbar = Snackbar.make(snackbarContainer, message, Snackbar.LENGTH_INDEFINITE)
        snackbar.setAction("Dismiss") { snackbar.dismiss() }
        snackbar.show()
    }

    private fun startMonitoring() {
        textView.visibility = View.VISIBLE
        handler.post(updateRunnable)
    }

    private fun stopMonitoring() {
        textView.visibility = View.GONE
        handler.removeCallbacks(updateRunnable)
    }

    external fun callNativeFunction(): String
    external fun getResourceUsage(): String

    companion object {
        init {
            System.loadLibrary("prekopiranceokod")
        }
    }
}
