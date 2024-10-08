package com.example.prekopiranceokod

import android.Manifest
import android.app.ActivityManager
import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Handler
import android.view.View
import android.widget.CheckBox
import android.widget.EditText
import android.widget.TextView
import android.content.Context
import android.os.Debug
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.google.android.material.snackbar.Snackbar
import java.io.File
import java.util.regex.Pattern
import android.view.inputmethod.EditorInfo
import android.view.inputmethod.InputMethodManager

class MainActivity : AppCompatActivity() {

    private lateinit var handler: Handler
    private lateinit var textView: TextView
    private lateinit var editTextProcessName: EditText
    private lateinit var editTextPid: EditText
    private lateinit var checkbox1: CheckBox
    private lateinit var checkbox2: CheckBox
    private lateinit var checkbox3: CheckBox
    private var isMonitoring = false
    private val updateInterval: Long = 1000 // 1 sekunda

    private val updateRunnable = object : Runnable {
        override fun run() {
            if (isMonitoring) {
                val processName = editTextProcessName.text.toString()
                val pidString = editTextPid.text.toString()
                val pid = if (pidString.isNotEmpty()) pidString.toInt() else -1 // Uzmi PID iz EditText-a

                try {
                    val resourceUsage: String = when {
                        checkbox1.isChecked && checkbox2.isChecked && checkbox3.isChecked -> {
                            getCpuUsageForProcessName(processName) + "\n" + getUsageForProcessPid(pid) + "\n" + getTotalCpuUsage()
                        }
                        checkbox1.isChecked && checkbox2.isChecked -> {
                            getCpuUsageForProcessName(processName) + "\n" + getTotalCpuUsage()
                        }
                        checkbox1.isChecked && checkbox3.isChecked -> {
                            getCpuUsageForProcessName(processName) + "\n" + getUsageForProcessPid(pid)
                        }
                        checkbox2.isChecked && checkbox3.isChecked -> {
                            getTotalCpuUsage() + "\n" + getUsageForProcessPid(pid)
                        }
                        checkbox1.isChecked -> {
                            getCpuUsageForProcessName(processName)
                        }
                        checkbox2.isChecked -> {
                            getTotalCpuUsage()
                        }
                        checkbox3.isChecked -> {
                            getUsageForProcessPid(pid)
                        }
                        else -> ""
                    }

                    if (pid != -1) {
                        val memoryUsage = getMemoryUsageForProcess(processName)
                        val coreCount = getCoreNum()
                        val coreInfo = "Broj jezgara: $coreCount"
                        val output = "$resourceUsage\nPodaci iz Activity Manager-a : $memoryUsage\n$coreInfo"

                        runOnUiThread {
                            textView.text = output
                            textView.visibility = View.VISIBLE
                        }
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
        editTextProcessName = findViewById(R.id.edit_text_process_name)
        editTextPid = findViewById(R.id.edit_text_pid) // Inicijalizuj novi EditText
        checkbox1 = findViewById(R.id.checkbox1)
        checkbox2 = findViewById(R.id.checkbox2)
        checkbox3 = findViewById(R.id.checkbox3)

        textView.visibility = View.GONE
        checkbox1.setOnCheckedChangeListener { _, _ -> handleCheckBoxes() }
        checkbox2.setOnCheckedChangeListener { _, _ -> handleCheckBoxes() }
        checkbox3.setOnCheckedChangeListener { _, _ -> handleCheckBoxes() }

        requestPermissions()

        // Postavite OnEditorActionListener
        editTextProcessName.setOnEditorActionListener { _, actionId, _ ->
            if (actionId == EditorInfo.IME_ACTION_DONE) {
                closeKeyboard(editTextProcessName)
                true
            } else {
                false
            }
        }

        editTextPid.setOnEditorActionListener { _, actionId, _ ->
            if (actionId == EditorInfo.IME_ACTION_DONE) {
                closeKeyboard(editTextPid)
                true
            } else {
                false
            }
        }
    }

    private fun closeKeyboard(view: View) {
        val imm = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        imm.hideSoftInputFromWindow(view.windowToken, 0)
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
        if (requestCode == 1 && grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            // Dozvola je data
        } else {
            showSnackbar("Dozvola nije data!")
        }
    }

    private fun showSnackbar(message: String) {
        val snackbarContainer: View = findViewById(R.id.snackbar_container)
        val snackbar = Snackbar.make(snackbarContainer, message, Snackbar.LENGTH_INDEFINITE)
        snackbar.setAction("Dismiss") { snackbar.dismiss() }
        snackbar.show()
    }

    private fun handleCheckBoxes() {
        if (checkbox1.isChecked || checkbox2.isChecked || checkbox3.isChecked) {
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

    private fun getCoreNum(): Int {
        var ret = 0
        try {
            val dir = File("/sys/devices/system/cpu/")
            val files = dir.listFiles { pathname ->
                Pattern.matches("cpu[0-9]+", pathname.name)
            }
            ret = files?.size ?: 0
        } catch (e: Exception) {
            ret = 1
        }
        return ret
    }

    private fun getMemoryUsageForProcess(processName: String): String {
        val pid = getPidForProcessName(processName)

        return if (pid != -1) {
            val activityManager = getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
            val memoryInfo = ActivityManager.MemoryInfo()
            activityManager.getMemoryInfo(memoryInfo)

            val debugMemoryInfo = Debug.MemoryInfo()
            Debug.getMemoryInfo(debugMemoryInfo)
            val pssMemory = debugMemoryInfo.getTotalPss() // PSS memorija u KB

            val totalMemory = memoryInfo.totalMem // Ukupna memorija u bajtovima
            val availableMemory = memoryInfo.availMem // Dostupna memorija u bajtovima

            val totalMemoryKB = totalMemory / 1024 // Ukupna memorija u kB
            val availableMemoryKB = availableMemory / 1024 // Dostupna memorija u kB

            "Zauzetost memorije za PID $pid: PSS: $pssMemory KB, Ukupna memorija: $totalMemoryKB kB, Dostupna memorija: $availableMemoryKB kB"
        } else {
            "PID za proces $processName nije pronađen."
        }
    }


    private fun getPidForProcessName(processName: String): Int {
        val activityManager = getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        val runningAppProcesses = activityManager.runningAppProcesses
        for (process in runningAppProcesses) {
            if (process.processName.equals(processName, ignoreCase = true)) {
                return process.pid
            }
        }
        return -1 // Ako PID nije pronađen
    }

    external fun getCpuUsageForProcessName(processName: String): String
    external fun getTotalCpuUsage(): String
    external fun getUsageForProcessPid(pid: Int): String

    companion object {
        init {
            System.loadLibrary("prekopiranceokod")
        }
    }
}
