package com.example.prekopiranceokod

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import com.example.prekopiranceokod.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        //setContentView(binding.root)
        setContentView(R.layout.activity_main)

        val button = findViewById<Button>(R.id.button)
        button.setOnClickListener {
            // Pozovi JNI metodu
            val result = callNativeFunction();// runCCode();
            Toast.makeText(this@MainActivity, "Native code result: $result", Toast.LENGTH_SHORT).show()
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