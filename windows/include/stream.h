#pragma once

#include <wx/wx.h>

#include <functional>
#include <vector>
#include <chrono>

#include "rtsp/receiver.h"

class Stream : public RTSP::Receiver::FrameReceivedListener
{
public:
	struct FrameStats
	{
		long long time;
		size_t size;
	};

	using OnFrameReadyCallback = std::function<void(const wxImage& image, FrameStats stats)>;

	struct Transforms
	{
		static const int ROTATE_0 = 0;
		static const int ROTATE_90 = 1;
		static const int ROTATE_180 = 2;
		static const int ROTATE_270 = 3;

		int rotation;
		bool mirror;
	};

	struct Adjustments
	{
		int brightness;
		int saturation;
		int quality;
		int whitebalance;
		int effect;
	};

	Stream(OnFrameReadyCallback fn);
	
	virtual void OnFrameReceived(unsigned char* bytes, int width, int height) const override;
	
	/// Rotates the image stream by 90 degrees counterclockwise
	int RotateLeft();

	/// Rotates the image stream by 90 degress clockwise
	int RotateRight();

	/// Mirror / flip the image on the horizontal axis
	void Mirror();

	void SetBrightnessAdjustment(int value);
	void SetSaturationAdjustment(int value);
	void SetQualityAdjustment(int value);
	void SetWBAdjustment(int value);
	void SetEffectAdjustment(int value);

	const Adjustments& GetAdjustments();

	wxImage ApplyFrameTransformsAndAdjustments() const;

	unsigned char* GetBGR(const wxImage& image);

	void Pause();
	void Unpause();
	void Close();

private:
	bool closed;
	bool paused;

	int bgrSize = 0;
	unsigned char* bgrData = nullptr;
	
	mutable wxImage image;
	OnFrameReadyCallback onFrameReady;

	Transforms transforms;
	Adjustments adjustments;

	mutable std::chrono::high_resolution_clock::time_point previousTimepoint;
};
