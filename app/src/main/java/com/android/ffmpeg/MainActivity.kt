package com.android.ffmpeg

import android.os.Bundle
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
import com.android.ffmpeg.ui.theme.FFmpegAndroidCodeTheme

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

        val textView = findViewById<TextView>(R.id.source_name)

        findViewById<View>(R.id.start).setOnClickListener {
            textView.text = getFFMpegVersion()

        }
    }


   private external fun getFFMpegVersion(): String

}
