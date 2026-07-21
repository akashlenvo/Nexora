package com.yvesgodoy.nexora.video.overlays;

import android.content.Context;
import android.graphics.Canvas
import android.graphics.Paint
import android.graphics.Rect
import android.util.AttributeSet;
import android.util.Size
import android.view.Gravity
import android.widget.LinearLayout
import android.widget.TextView
import com.yvesgodoy.nexora.R

class ScannerOverlay(context: Context, attrs: AttributeSet) : LinearLayout(context, attrs) {

    private val widthPercentage = 0.7f
    private val heightPercentage = 0.35f
    private val cornerSize = 150
    public val rect = Rect()
    public lateinit var size: Size

    private val paint = Paint().apply {
        color = 0xFFFFFFFF.toInt() // White border
        strokeWidth = 8f
        style = Paint.Style.STROKE
        isAntiAlias = true
    }

    private val textView: TextView = TextView(context).apply {
        textSize = 18f
        gravity = Gravity.CENTER
        setTextColor(0xFFFFFFFF.toInt())
        layoutParams = LayoutParams(
            LayoutParams.WRAP_CONTENT,
            LayoutParams.WRAP_CONTENT
        ).apply {
            gravity = Gravity.CENTER
        }
    }

    init {
        orientation = VERTICAL
        context.theme.obtainStyledAttributes(attrs, R.styleable.ScannerOverlay, 0, 0).apply {
            try {
                textView.text = getString(R.styleable.ScannerOverlay_text) ?: ""
            } finally {
                recycle()
            }
        }

        addView(textView)
    }

    override fun dispatchDraw(canvas: Canvas) {
        super.dispatchDraw(canvas)
        size = Size(width, height)

        val left = (width - width * widthPercentage) / 2
        val top = (height - height * heightPercentage) / 2
        val right = width - left
        val bottom = height - top
        rect.set(left.toInt(), top.toInt(), right.toInt(), bottom.toInt())

        // Top left
        canvas.drawArc(left, top, left + cornerSize, top + cornerSize, -180f, 90f, false, paint)
        // Top right
        canvas.drawArc(right - cornerSize, top, right, top + cornerSize, 0f, -90f, false, paint)
        // Bottom right
        canvas.drawArc(right - cornerSize, bottom - cornerSize, right, bottom, 0f, 90f, false, paint)
        // Bottom left
        canvas.drawArc(left, bottom-cornerSize, left + cornerSize, bottom, -180f, -90f, false, paint)

        textView.y = top - 200
        textView.maxWidth = (width * widthPercentage).toInt() + cornerSize
    }
}
