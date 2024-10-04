package com.example.prekopiranceokod // Izmenjen naziv paketa

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Handler
import android.view.View
import android.widget.CheckBox
import android.widget.EditText
import android.widget.TextView
import android.app.ActivityManager
import android.content.Context
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.google.android.material.snackbar.Snackbar
import java.io.BufferedReader
import java.io.InputStreamReader
import java.io.File
import java.util.regex.Pattern
import java.io.RandomAccessFile
import android.os.Debug
import kotlin.concurrent.thread



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
                val processName = editTextProcessName.text.toString() // Preuzmite ime procesa
                try {
                    // Prvo dobijemo PID za proces
                    val pid = 9780 // getPidForProcessName(processName)

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

                    // CPU zauzetost za PID
                    if (pid != -1) {
                        calculateCpuUsage(this@MainActivity, pid) { cpuUsageForPid ->
                            // Dodajte ispis CPU usage za PID
                            val cpuInfo = "Zauzetost CPU za PID $pid: $cpuUsageForPid%"

                            // Dodajte poziv za zauzetost memorije
                            val memoryUsage = getMemoryUsageForProcess(processName)

                            // Dodajte poziv za broj jezgara
                            val coreCount = getCoreNum()
                            val coreInfo = "Broj jezgara: $coreCount "

                            // Spojite rezultate
                            val output = "$resourceUsage\n$memoryUsage\n$coreInfo $cpuInfo"

                            // Osvežavanje UI na glavnom threadu
                            runOnUiThread {
                                textView.text = output
                                textView.visibility = View.VISIBLE
                            }
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



    // Klasa za informacije o procesu
    data class ProcessInfo(
        val pid: Int,
        val userTime: Long,   // CPU vreme potrošeno u korisničkom modu
        val systemTime: Long  // CPU vreme potrošeno u sistemskom modu
    )

    // Funkcija za dobijanje informacija o procesu
    fun getProcessInfo(context: Context, pid: Int): ProcessInfo? {
        val activityManager = context.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        val runningAppProcesses = activityManager.runningAppProcesses

        for (process in runningAppProcesses) {
            if (process.pid == pid) {
                // Nađi informacije o CPU vremenu
                val processInfo = android.os.Process.getElapsedCpuTime()
                return ProcessInfo(pid, processInfo, processInfo)
            }
        }
        return null
    }

    // Funkcija za izračunavanje zauzetosti CPU-a u prethodnoj sekundi
    fun getCpuUsageForLastSecond(context: Context, pid: Int): Double {
        // Prvo izmerimo CPU vreme pre
        val initialProcessInfo = getProcessInfo(context, pid) ?: return 0.0

        // Sačekaj 1 sekundu
        Thread.sleep(1000)

        // Izmerimo CPU vreme posle
        val finalProcessInfo = getProcessInfo(context, pid) ?: return 0.0

        // Izračunamo razliku u CPU vremenu
        val userTimeDiff = finalProcessInfo.userTime - initialProcessInfo.userTime
        val systemTimeDiff = finalProcessInfo.systemTime - initialProcessInfo.systemTime
        val totalTimeDiff = userTimeDiff + systemTimeDiff

        // Ukupno vreme procesora je 1 sekunda (1000 ms)
        val cpuUsage = (totalTimeDiff.toDouble() / 1000.0) * 100 // Izrazimo kao procenat

        return cpuUsage
    }

    // Callback funkcija koja prima CPU zauzetost kao argument
    fun calculateCpuUsage(activity: Context, pid: Int, callback: (Double) -> Unit) {
        // Pokreni novu nit
        thread {
            // Izračunaj CPU zauzetost
            val cpuUsage = getCpuUsageForLastSecond(activity, pid)
            Log.d("CPU Usage", "CPU usage for process $pid in last second: $cpuUsage%")

            // Pozovi callback sa izračunatom vrednošću
            callback(cpuUsage)
        }
    }



// U svom Activity-ju možeš pozvati calculateCpuUsage i proslediti PID
// Na primer: calculateCpuUsage(this, Process.myPid()) // Da dobiješ za trenutni proces



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
        val pid = 9780//getPidForProcessName(processName)

        return if (pid != -1) {
            val activityManager = getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
            val memoryInfo = ActivityManager.MemoryInfo()
            activityManager.getMemoryInfo(memoryInfo)

            val totalMemory = memoryInfo.totalMem / 1024 // Ukupna memorija u KB
            val availableMemory = memoryInfo.availMem / 1024 // Dostupna memorija u KB
            val usedMemory = totalMemory - availableMemory // Zauzeta memorija u KB


            // Koristimo Debug.MemoryInfo za dobijanje RSS memorije
            val debugMemoryInfo = Debug.MemoryInfo()
            Debug.getMemoryInfo(debugMemoryInfo)
            val rssMemory = debugMemoryInfo.getTotalPss()
                //debugMemoryInfo.getTotalPrivateDirty() // RSS u KB


            "Zauzetost memorije za PID $pid: RSS: $rssMemory KB, Ukupna memorija: $totalMemory KB, Dostupna memorija: $availableMemory KB"
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
