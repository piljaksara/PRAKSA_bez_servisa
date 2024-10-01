package com.example.prekopiranceokod
import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Handler
import android.view.View
import android.widget.Button
import android.widget.CheckBox
import android.widget.EditText
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.google.android.material.snackbar.Snackbar

class MainActivity : AppCompatActivity() {

    private lateinit var handler: Handler
    private lateinit var textView: TextView
    private lateinit var editTextPid: EditText
    private var isMonitoring = false
    private val updateInterval: Long = 1000 // 1 sekunda

    private val updateRunnable = object : Runnable {
        override fun run() {
            if (isMonitoring) {
                // Pozovite JNI funkciju koja vraća informacije
                val pid = editTextPid.text.toString()
                val result = getResourceUsage(pid)
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
        editTextPid = findViewById(R.id.edit_text_pid)

        // Inicialno postavljanje TextView na nevidljiv
        textView.visibility = View.GONE

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

        // Traženje dozvola
        requestPermissions()
    }

    private fun requestPermissions() {
        val permissions = arrayOf(
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.ACCESS_COARSE_LOCATION
        )

        ActivityCompat.requestPermissions(this, permissions, 1)
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == 1) {
            if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                // Dozvola je data
            } else {
                // Dozvola nije data
                showSnackbar("Dozvola nije data!")
            }
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

    // Dodaj parametar pid za JNI funkciju
    external fun callNativeFunction(): String
    external fun getResourceUsage(pid: String): String

    companion object {
        init {
            System.loadLibrary("prekopiranceokod")
        }
    }
}
