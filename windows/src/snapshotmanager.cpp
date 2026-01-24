// Allow the use of unsafe marked functions 
// like fopen, strcpy, etc (C standard functions)
// used by wxWidgets internally (e.g. wxStandardPaths).
#define _CRT_SECURE_NO_WARNINGS

#include "snapshotmanager.h"
#include "settings.h"
#include "logger.h"

#include <thread>
#include <wx/image.h>
#include <wx/stdpaths.h>

SnapshotManager::SnapshotManager() : scaler(AV_PIX_FMT_RGB24), snapshotRequested(false)
{
}

void SnapshotManager::RequestSnapshot()
{
	snapshotRequested = true;
}

void SnapshotManager::ProcessFrame(AVFrame* frame)
{
	if (!snapshotRequested.load(std::memory_order_relaxed))
		return;

	if (snapshotRequested.exchange(false))
	{
		int width = 0, height = 0;

		const uint8_t* frameData = scaler.Process(frame, width, height);

		if (!frameData || width <= 0 || height <= 0)
			return;

		size_t sz = width * height * 3;
		uint8_t* dataCopy = new uint8_t[sz];

		if (dataCopy)
		{
			std::memcpy(dataCopy, frameData, sz);
			std::thread(&SnapshotManager::SaveSnapshot, this, dataCopy, width, height).detach();
		}
	}
}

void SnapshotManager::SaveSnapshot(uint8_t* data, int width, int height)
{
	wxImage img(width, height, data, true);

	wxString filename = wxDateTime::Now().Format("Snapshot_%Y%m%d_%H%M%S.jpg");
	wxFileName path = Settings::GetDocumentsDirectoryPath();
	path.AppendDir("Snapshots");
	path.SetFullName(filename);

	if (img.SaveFile(path.GetFullPath(), wxBITMAP_TYPE_JPEG))
		logger << "Snapshot saved to " << path.GetFullPath().ToUTF8() << std::endl;
	else
		logger << "Failed to save snapshot to " << path.GetFullPath().ToUTF8() << std::endl;
}
