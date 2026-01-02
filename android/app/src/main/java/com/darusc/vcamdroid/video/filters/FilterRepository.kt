package com.darusc.vcamdroid.video.filters

import com.pedro.encoder.input.gl.render.filters.BaseFilterRender
import com.pedro.encoder.input.gl.render.filters.*

/**
 * Repository of all available OpenGL filters provided by pedro's RootEncoder library
 */
object FilterRepository {

    enum class Category {
        NONE, CORRECTION, EFFECT, DISTORTION, ARTISTIC
    }

    /**
     * Filter item, mapping the filter name to the actual class
     */
    data class FilterInfo(
        val name: String,
        val filterClass: Class<out BaseFilterRender>,
        val category: Category
    ) {
        override fun toString() = name
    }

    fun getCategory(filterName: String): Category =
        filters.find { it.name.equals(filterName, ignoreCase = true) }?.let {
            it.category
        } ?: Category.NONE

    fun getClass(filterName: String): Class<out BaseFilterRender>? =
        filters.find { it.name.equals(filterName, ignoreCase = true) }?.filterClass

    fun create(filterName: String): BaseFilterRender? =
        filters.find { it.name.equals(filterName, ignoreCase = true) }?.let { create(it) }

    fun create(filterInfo: FilterInfo): BaseFilterRender? {
        return try {
            filterInfo.filterClass.getDeclaredConstructor().newInstance()
        } catch (e: Exception) {
            e.printStackTrace()
            null
        }
    }

    val filters: List<FilterInfo> = listOf(
        // --- Default ---
        FilterInfo("None", NoFilterRender::class.java, Category.NONE),

        // --- Corrections ---
        FilterInfo("Brightness", BrightnessFilterRender::class.java, Category.CORRECTION),
        FilterInfo("Contrast", ContrastFilterRender::class.java, Category.CORRECTION),
        FilterInfo("Exposure", ExposureFilterRender::class.java, Category.CORRECTION),
        FilterInfo("Gamma", GammaFilterRender::class.java, Category.CORRECTION),
        FilterInfo("Saturation", SaturationFilterRender::class.java, Category.CORRECTION),
        FilterInfo("Temperature", TemperatureFilterRender::class.java, Category.CORRECTION),
        FilterInfo("Sharpness", SharpnessFilterRender::class.java, Category.CORRECTION),

        // --- Classic Effects ---
        FilterInfo("Grey Scale", GreyScaleFilterRender::class.java, Category.EFFECT),
        FilterInfo("Sepia", SepiaFilterRender::class.java, Category.EFFECT),
        FilterInfo("Negative", NegativeFilterRender::class.java, Category.EFFECT),
        FilterInfo("Early Bird", EarlyBirdFilterRender::class.java, Category.EFFECT),
        FilterInfo("70s Style", Image70sFilterRender::class.java, Category.EFFECT),
        FilterInfo("Lamoish", LamoishFilterRender::class.java, Category.EFFECT),

        // --- Artistic ---
        FilterInfo("Cartoon", CartoonFilterRender::class.java, Category.ARTISTIC),
        FilterInfo("Duotone", DuotoneFilterRender::class.java, Category.ARTISTIC),
        FilterInfo("Halftone Lines", HalftoneLinesFilterRender::class.java, Category.ARTISTIC),
        FilterInfo("Pixelated", PixelatedFilterRender::class.java, Category.ARTISTIC),
        FilterInfo("Polygonization", PolygonizationFilterRender::class.java, Category.ARTISTIC),
        FilterInfo("Rainbow", RainbowFilterRender::class.java, Category.ARTISTIC),
        FilterInfo("Money", MoneyFilterRender::class.java, Category.ARTISTIC),
        FilterInfo("Zebra", ZebraFilterRender::class.java, Category.ARTISTIC),
        FilterInfo("Edge Detection", EdgeDetectionFilterRender::class.java, Category.ARTISTIC),

        // --- Distortion & FX ---
        FilterInfo("Blur", BlurFilterRender::class.java, Category.DISTORTION),
        FilterInfo("Glitch", GlitchFilterRender::class.java, Category.DISTORTION),
        FilterInfo("Noise", NoiseFilterRender::class.java, Category.DISTORTION),
        FilterInfo("Analog TV", AnalogTVFilterRender::class.java, Category.DISTORTION),
        FilterInfo("Swirl", SwirlFilterRender::class.java, Category.DISTORTION),
        FilterInfo("Ripple", RippleFilterRender::class.java, Category.DISTORTION),
        // FilterInfo("Basic Deformation", BasicDeformationFilterRender::class.java, Category.DISTORTION),
        FilterInfo("Fire", FireFilterRender::class.java, Category.DISTORTION),
        FilterInfo("Snow", SnowFilterRender::class.java, Category.DISTORTION),

        // --- Shapes & Utility ---
//        Filter("Circle", CircleFilterRender::class.java),
//        Filter("Crop", CropFilterRender::class.java),
//        Filter("Rotation", RotationFilterRender::class.java),
//        Filter("Chroma Key", ChromaFilterRender::class.java),
//        Filter("Android View", AndroidViewFilterRender::class.java)
    )
}