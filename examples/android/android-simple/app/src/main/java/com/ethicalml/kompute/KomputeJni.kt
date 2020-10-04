/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.ethicalml.kompute

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import android.widget.Toast
import android.view.View
import android.widget.EditText
import android.widget.TextView
import com.ethicalml.kompute.databinding.ActivityKomputeJniBinding

class KomputeJni : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val binding = ActivityKomputeJniBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.komputeGifView.loadUrl("file:///android_asset/komputer-2.gif")

        binding.komputeGifView.getSettings().setUseWideViewPort(true)
        binding.komputeGifView.getSettings().setLoadWithOverviewMode(true)

        val successVulkanInit = initVulkan()
        if (successVulkanInit) {
            Toast.makeText(applicationContext, "Vulkan Loaded SUCCESS", Toast.LENGTH_SHORT).show()
        } else {
            binding.KomputeButton.isEnabled = false
            Toast.makeText(applicationContext, "Vulkan Load FAILED", Toast.LENGTH_SHORT).show()
        }
        Log.i("KomputeJni", "Vulkan Result: " + successVulkanInit)

        binding.komputeJniTextview.text = "N/A"
    }

    fun KomputeButtonOnClick(v: View) {

        val xiEditText = findViewById<EditText>(R.id.XIEditText)
        val xjEditText = findViewById<EditText>(R.id.XJEditText)
        val yEditText = findViewById<EditText>(R.id.YEditText)
        val komputeJniTextview = findViewById<TextView>(R.id.kompute_jni_textview)

        val xi = xiEditText.text.removeSurrounding("[", "]").split(",").map { it.toFloat() }.toFloatArray()
        val xj = xjEditText.text.removeSurrounding("[", "]").split(",").map { it.toFloat() }.toFloatArray()
        val y = yEditText.text.removeSurrounding("[", "]").split(",").map { it.toFloat() }.toFloatArray()

        val out = kompute(xi, xj, y)

        Log.i("KomputeJni", "RESULT:")
        Log.i("KomputeJni", out.contentToString())

        komputeJniTextview.text = out.contentToString()
    }

    external fun initVulkan(): Boolean

    external fun kompute(xi: FloatArray, xj: FloatArray, y: FloatArray): FloatArray

    companion object {
        init {
            System.loadLibrary("kompute-jni")
        }
    }
}

