#pragma once

#include <wx/wx.h>
#include "canvas.h"

#include "qrcodegen.hpp"

class QrconView : public wxDialog
{
public:
	QrconView(std::string name, std::string address, std::string port, wxSize displaySize);

private:
	qrcodegen::QrCode GenerateQRCode(std::string data);
	wxImage GenerateImageFromQR(const qrcodegen::QrCode& qrcode);
};