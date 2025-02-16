package com.android.ffmpeg

import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.View
import android.widget.TextView
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.android.ffmpeg.ui.theme.FFmpegAndroidCodeTheme
import java.io.File
import java.io.FileOutputStream
import java.io.IOException

class MainActivity : ComponentActivity() {

    companion object {
         init {
             System.loadLibrary("decodeMp4")
        }

    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        setContentView(R.layout.activity_main)

        if (ContextCompat.checkSelfPermission(this,
                android.Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, arrayOf(android.Manifest.permission.WRITE_EXTERNAL_STORAGE), 101)
        }

        if (ContextCompat.checkSelfPermission(this,
                android.Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, arrayOf(android.Manifest.permission.READ_EXTERNAL_STORAGE), 101)
        }


        val textView = findViewById<TextView>(R.id.source_name)

        val path = filesDir.path
        Log.i("mainActivity", "src path is $path")

        findViewById<View>(R.id.start).setOnClickListener {
            textView.text = getFFMpegVersion()

//            val testPath = getExternalFilesDir(null)
//            val file = File(testPath, "log.txt")
//            Log.d("file-", "file path is ${file.path}")
//            try {
//                // 创建文件并写入数据
//                val fos = FileOutputStream(file)
//                fos.write("123456".toByteArray())
//                Log.d("file-", "write end...")
//                fos.close()
//            } catch (e: IOException) {
//                e.printStackTrace()
//            }

            val path = filesDir.path
            Log.i("mainActivity", "src path is $path/sample.mp4")
            val result = decodeMp4ToYuv("$path/sample.mp4",
                "$path/sample2.yuv");
//
            Log.i("mainActivity", "result is $result");

        }
    }


    private external fun decodeMp4ToYuv(sourceName: String, dstName: String): Int


    private external fun getFFMpegVersion(): String

}
