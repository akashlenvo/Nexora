package com.yvesgodoy.nexora.video.overlays

import android.content.Context
import android.graphics.Canvas
import android.graphics.Paint
import android.graphics.Rect
import android.util.AttributeSet
import android.util.Size
import android.view.View

class GridOverlay(context: Context, attrs: AttributeSet) : View(context, attrs) {

    private val paint = Paint().apply {
        color = 0x80FFFFFF.toInt()
        strokeWidth = 2f
        style = Paint.Style.STROKE
    }

    @Override
    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        val w = width.toFloat()
        val h = height.toFloat()

        for (i in 1..3) {
            val x = i * w / 3
            val y = i * h / 3
            canvas.drawLine(x, 0f, x, h, paint)
            canvas.drawLine(0f, y, w, y, paint)
        }
    }
}