package com.darusc.vcamdroid.video.filters

import com.pedro.encoder.input.gl.render.filters.*

private typealias Adjuster = (BaseFilterRender, Float) -> Unit

object FilterAdjuster {

    /**
     * @param filter The base filter to adjust
     * @param value The adjustment 0-100
     */
    fun adjust(filter: BaseFilterRender, value: Int) {
        val normalized = value / 100f
        val adjuster = registry[filter::class.java]

        adjuster?.invoke(filter, normalized)
    }

    private val registry = mapOf<Class<out BaseFilterRender>, Adjuster>(

        BrightnessFilterRender::class.java to { f, p ->
            (f as BrightnessFilterRender).brightness = (p * 2) - 1
        },

        ContrastFilterRender::class.java to { f, p ->
            (f as ContrastFilterRender).contrast = p * 2
        },

        ExposureFilterRender::class.java to { f, p ->
            (f as ExposureFilterRender).exposure = (p * 2) - 1
        },

        GammaFilterRender::class.java to { f, p ->
            (f as GammaFilterRender).gamma = p * 2
        },

        SaturationFilterRender::class.java to { f, p ->
            (f as SaturationFilterRender).saturation = p * 2
        },

        TemperatureFilterRender::class.java to { f, p ->
            (f as TemperatureFilterRender).setTemperature(p)
        },

        SharpnessFilterRender::class.java to { f, p ->
            (f as SharpnessFilterRender).sharpness = p
        },
    )
}