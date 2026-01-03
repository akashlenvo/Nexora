package com.darusc.vcamdroid.video.filters.custom;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.Matrix;
import android.os.Build;

import androidx.annotation.RequiresApi;

import com.pedro.encoder.R;
import com.pedro.encoder.input.gl.render.filters.BaseFilterRender;
import com.pedro.encoder.utils.gl.GlUtil;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * A filter that flips the image vertically by modifying texture coordinates.
 */
@RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
public class VerticalFlipFilterRender extends BaseFilterRender {

    // We modify the V (Vertical) texture coordinates here (the 5th element in each row).
    // Standard V is 0.0 (bottom) -> 1.0 (top). We swap them to 1.0 -> 0.0 to flip vertically.
    private final float[] squareVertexDataFilter = {
            // X, Y, Z, U, V
            -1f, -1f, 0f, 0f, 1f, // bottom left  (Original V was 0f, changed to 1f)
            1f, -1f, 0f, 1f, 1f, // bottom right (Original V was 0f, changed to 1f)
            -1f, 1f, 0f, 0f, 0f, // top left     (Original V was 1f, changed to 0f)
            1f, 1f, 0f, 1f, 0f, // top right    (Original V was 1f, changed to 0f)
    };

    private int program = -1;
    private int aPositionHandle = -1;
    private int aTextureHandle = -1;
    private int uMVPMatrixHandle = -1;
    private int uSTMatrixHandle = -1;
    private int uSamplerHandle = -1;

    public VerticalFlipFilterRender() {
        squareVertex = ByteBuffer.allocateDirect(squareVertexDataFilter.length * FLOAT_SIZE_BYTES)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        squareVertex.put(squareVertexDataFilter).position(0);
        Matrix.setIdentityM(MVPMatrix, 0);
        Matrix.setIdentityM(STMatrix, 0);
    }

    @Override
    protected void initGlFilter(Context context) {
        // Use the basic shaders since we are only manipulating geometry/coordinates, not colors.
        String vertexShader = GlUtil.getStringFromRaw(context, R.raw.simple_vertex);
        String fragmentShader = GlUtil.getStringFromRaw(context, R.raw.simple_fragment);

        program = GlUtil.createProgram(vertexShader, fragmentShader);
        aPositionHandle = GLES20.glGetAttribLocation(program, "aPosition");
        aTextureHandle = GLES20.glGetAttribLocation(program, "aTextureCoord");
        uMVPMatrixHandle = GLES20.glGetUniformLocation(program, "uMVPMatrix");
        uSTMatrixHandle = GLES20.glGetUniformLocation(program, "uSTMatrix");
        uSamplerHandle = GLES20.glGetUniformLocation(program, "uSampler");
    }

    @Override
    protected void drawFilter() {
        GLES20.glUseProgram(program);

        squareVertex.position(SQUARE_VERTEX_DATA_POS_OFFSET);
        GLES20.glVertexAttribPointer(aPositionHandle, 3, GLES20.GL_FLOAT, false,
                SQUARE_VERTEX_DATA_STRIDE_BYTES, squareVertex);
        GLES20.glEnableVertexAttribArray(aPositionHandle);

        squareVertex.position(SQUARE_VERTEX_DATA_UV_OFFSET);
        GLES20.glVertexAttribPointer(aTextureHandle, 2, GLES20.GL_FLOAT, false,
                SQUARE_VERTEX_DATA_STRIDE_BYTES, squareVertex);
        GLES20.glEnableVertexAttribArray(aTextureHandle);

        GLES20.glUniformMatrix4fv(uMVPMatrixHandle, 1, false, MVPMatrix, 0);
        GLES20.glUniformMatrix4fv(uSTMatrixHandle, 1, false, STMatrix, 0);

        GLES20.glUniform1i(uSamplerHandle, 0);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, previousTexId);
    }

    @Override
    protected void disableResources() {
        GlUtil.disableResources(aTextureHandle, aPositionHandle);
    }

    @Override
    public void release() {
        GLES20.glDeleteProgram(program);
    }
}