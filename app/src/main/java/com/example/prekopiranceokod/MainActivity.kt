package com.example.prekopiranceokod

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Handler
import android.view.View
import android.widget.CheckBox
import android.widget.EditText
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.google.android.material.snackbar.Snackbar

class MainActivity : AppCompatActivity() {

    private lateinit var handler: Handler
    private lateinit var textView: TextView
    private lateinit var editTextProcessName: EditText // Izmenjeno ime varijable
    private lateinit var checkbox1: CheckBox
    private lateinit var checkbox2: CheckBox
    private var isMonitoring = false
    private val updateInterval: Long = 1000 // 1 sekunda

    private val updateRunnable = object : Runnable {
        override fun run() {
            if (isMonitoring) {
                val processName = editTextProcessName.text.toString() // Promenite ovo da preuzmete ime procesa
                // Preuzmi CPU i ukupnu zauzetost unutar try-catch blokova da se uhvate greške
                try {
                    val resourceUsage: String = when {
                        checkbox1.isChecked && checkbox2.isChecked -> {
                            getCpuUsageForProcessName(processName) + "\n" + getTotalCpuUsage() // Ažurirajte poziv
                        }
                        checkbox1.isChecked -> {
                            getCpuUsageForProcessName(processName) // Ažurirajte poziv
                        }
                        checkbox2.isChecked -> {
                            getTotalCpuUsage()
                        }
                        else -> ""
                    }

                    // Osvežavanje UI na glavnom threadu
                    runOnUiThread {
                        textView.text = resourceUsage
                        textView.visibility = View.VISIBLE
                    }
                } catch (e: Exception) {
                    showSnackbar("Došlo je do greške: ${e.message}")
                }
                handler.postDelayed(this, updateInterval)
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        handler = Handler()
        textView = findViewById(R.id.text_view)
        editTextProcessName = findViewById(R.id.edit_text_process_name) // Izmenjeno ime ID-a
        checkbox1 = findViewById(R.id.checkbox1)
        checkbox2 = findViewById(R.id.checkbox2)

        // Inicialno postavljanje TextView na nevidljiv
        textView.visibility = View.GONE

        checkbox1.setOnCheckedChangeListener { _, _ -> handleCheckBoxes() }
        checkbox2.setOnCheckedChangeListener { _, _ -> handleCheckBoxes() }

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

    private fun handleCheckBoxes() {
        // Start/stop monitoring based on the checkboxes
        if (checkbox1.isChecked || checkbox2.isChecked) {
            startMonitoring()
        } else {
            stopMonitoring()
        }
    }

    private fun startMonitoring() {
        textView.visibility = View.VISIBLE
        handler.post(updateRunnable)
        isMonitoring = true
    }

    private fun stopMonitoring() {
        textView.visibility = View.GONE
        handler.removeCallbacks(updateRunnable)
        isMonitoring = false
    }

    // Dodaj parametar pid za JNI funkciju
    external fun callNativeFunction(): String
    external fun getResourceUsage(pid: String): String
    external fun getCpuUsageForProcessName(processName: String): String // Dodaj ovu metodu u JNI
    external fun getTotalCpuUsage(): String // Dodaj ovu metodu u JNI

    companion object {
        init {
            System.loadLibrary("prekopiranceokod")
        }
    }
}
