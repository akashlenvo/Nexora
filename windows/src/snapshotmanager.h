#pragma once

#include "video/1_1scaler.h"
#include <atomic>

class SnapshotManager
{
public:
	SnapshotManager();

	/*
		Flags the manager to take a snaphot on the next processed frame
	*/
	void RequestSnapshot();

	/*
		Process an incoming frame for snapshotting if requested
	*/
	void ProcessFrame(AVFrame* frame);

private:
	Video::_1_1Scaler scaler;
	std::atomic<bool> snapshotRequested;

	void SaveSnapshot(uint8_t* data, int width, int height);
};