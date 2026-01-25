#pragma once

#include "rtsp/ffmpeg.h"
#include <vector>

namespace Video
{
	class FrameScaler
	{
	public:
		/// <param name="format">The output format of the processed frame</param>
		FrameScaler(int format);
		virtual ~FrameScaler();

		/// <summary>
		/// Process an incoming ffmpeg frame and output pixel data in the format of this scaler
		/// </summary>
		/// <param name="srcFrame"></param>
		/// <param name="width">Resulting width of the processed frame</param>
		/// <param name="height">Resulting height of the processed frame</param>
		/// <returns></returns>
		const uint8_t* Process(const AVFrame* srcFrame, int& width, int& height);

	protected:
		struct TargetDimensions
		{
			// The size of the video frame after being scaled
			int width, height;

			// The size of the output buffer. Might be bigger than the actual
			// frame, in that case black bars are present
			int containerWidth, containerHeight;

			// Starting coordinates (top-left) to draw the image
			int xOffset, yOffset;

			// Flag indicating if black bars are present
			bool hasPadding;
		};

		/// <summary>
		/// Calculate the target dimensions for the processed frame.
		/// Subclasses must implement this to accomodate their needs.
		/// </summary>
		/// <param name="width">Incoming frame's width</param>
		/// <param name="height">Incoming frame's height</param>
		/// <returns></returns>
		virtual TargetDimensions CalculateTargetDimensions(int width, int height) = 0;

	private:
		int format;
		SwsContext* sws_ctx = nullptr;
		std::vector<uint8_t> buffer;

		TargetDimensions cachedTargetDims;
		int lastSrcWidth, lastSrcHeight, lastSrcFormat;

		bool PrepareContext(int srcWidth, int srcHeight, int srcFormat);
		void Cleanup();

		int GetBytesPerPixel(int fmt);
	};
};