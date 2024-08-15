package com.example.prekopiranceokod

import android.os.Bundle
import android.os.Handler
import android.view.View
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.snackbar.Snackbar
import com.example.prekopiranceokod.databinding.ActivityMainBinding
import android.widget.TextView

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private val handler = Handler()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val buttonExisting = findViewById<Button>(R.id.button_existing)
        buttonExisting.setOnClickListener {
            // Pozovi JNI metodu za postojeće dugme
            val result = callNativeFunction()

            // Prikazivanje Snackbar-a sa rezultatom
            showSnackbar(result)
        }

        val buttonNew = findViewById<Button>(R.id.button_new)
        buttonNew.setOnClickListener {
            // Pozovi funkciju za prelazak između stanja monitoringa
            val result = getResourceUsage()

            // Prikazivanje Snackbar-a sa rezultatom
            showSnackbar(result)
        }
    }


    private fun showSnackbar(message: String) {
        val snackbarContainer: View = findViewById(R.id.snackbar_container)
        val snackbar = Snackbar.make(snackbarContainer, " $message", Snackbar.LENGTH_INDEFINITE)

        // Primenite prilagođeni stil
        val snackbarLayout = snackbar.view as Snackbar.SnackbarLayout
        snackbarLayout.setBackgroundColor(resources.getColor(R.color.snackbar_background, null))

        val textView = snackbarLayout.findViewById<TextView>(com.google.android.material.R.id.snackbar_text)
        textView.setTextColor(resources.getColor(R.color.snackbar_text, null))
        textView.textSize = 18f // Postavite veličinu teksta

        snackbar.show()

        // Sakrij Snackbar nakon 5 sekundi
        handler.postDelayed({
            snackbar.dismiss()
        }, 5000) // 5000 ms = 5 sekundi
    }

    external fun callNativeFunction(): String
    external fun getResourceUsage(): String

    companion object {
        // Used to load the 'prekopiranceokod' library on application startup.
        init {
            System.loadLibrary("prekopiranceokod")
        }
    }
    private fun toggleMonitoring() {
        val isMonitoring = isMonitoringNative()  // Pretpostavljamo da imate funkciju koja vraća trenutni status monitoringa
        if (isMonitoring) {
            // Zaustavi monitoring
            stopMonitoring()
            showSnackbar("Monitoring stopped")
        } else {
            // Pokreni monitoring
            startMonitoring()
            showSnackbar("Monitoring started")

        }
    }

    private fun startMonitoring() {
        // Poziva JNI funkciju koja pokreće periodično ispisivanje
        startMonitoringNative()
    }

    private fun stopMonitoring() {
        // Poziva JNI funkciju koja zaustavlja periodično ispisivanje
        stopMonitoringNative()
    }

    // Pretpostavljamo da ove JNI funkcije postoje u vašem kodu
    external fun startMonitoringNative()
    external fun stopMonitoringNative()
    external fun isMonitoringNative(): Boolean

}
