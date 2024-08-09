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

        val button = findViewById<Button>(R.id.button)
        button.setOnClickListener {
            // Pozovi JNI metodu
            val result = callNativeFunction()

            // Prikazivanje Snackbar-a sa rezultatom
            val snackbarContainer: View = findViewById(R.id.snackbar_container)
            val snackbar = Snackbar.make(snackbarContainer, "Native code result: $result", Snackbar.LENGTH_INDEFINITE)

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
    }

    /**
     * A native method that is implemented by the 'prekopiranceokod' native library,
     * which is packaged with this application.
     */
    external fun callNativeFunction(): String

    companion object {
        // Used to load the 'prekopiranceokod' library on application startup.
        init {
            System.loadLibrary("prekopiranceokod")
        }
    }
}
