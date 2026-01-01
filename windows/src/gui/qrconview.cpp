#include "gui/qrconview.h"

QrconView::QrconView(std::string name, std::string address, std::string port, wxSize displaySize) : wxDialog(nullptr, wxID_ANY, "Connect with QR code")
{
	auto sizer = new wxBoxSizer(wxVERTICAL);
	auto wrapper = new wxBoxSizer(wxVERTICAL);

	auto qrcode = GenerateQRCode(address + ":" + port);
	auto image = GenerateImageFromQR(qrcode);

	wxStaticBitmap* canvas = new wxStaticBitmap(this, wxID_ANY, wxBitmap(image.Scale(displaySize)));

	wrapper->Add(new wxStaticText(this, wxID_ANY, "Scan the QR to connect automatically"), 0, wxALIGN_CENTER | wxTOP, FromDIP(30));
	wrapper->Add(canvas, 0, wxALIGN_CENTER | wxALL, FromDIP(15));
	wrapper->Add(new wxStaticText(this, wxID_ANY, wxString::Format("%s", name)), 0, wxALIGN_CENTER | wxBOTTOM, FromDIP(2));
	wrapper->Add(new wxStaticText(this, wxID_ANY, wxString::Format("Address: %s", address)), 0, wxALIGN_CENTER);
	wrapper->Add(new wxStaticText(this, wxID_ANY, wxString::Format("Port: %s", port)), 0, wxALIGN_CENTER | wxBOTTOM, FromDIP(30));

	sizer->Add(wrapper, 1, wxALIGN_CENTER | wxLEFT | wxRIGHT, FromDIP(50));

	this->SetSizerAndFit(sizer);
}

qrcodegen::QrCode QrconView::GenerateQRCode(std::string data)
{
	return qrcodegen::QrCode::encodeText(data.c_str(), qrcodegen::QrCode::Ecc::MEDIUM);
}

wxImage QrconView::GenerateImageFromQR(const qrcodegen::QrCode& qrcode)
{
	auto size = qrcode.getSize();

	unsigned char* bytes = new unsigned char[size * size * 3];

	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			unsigned char value = qrcode.getModule(x, y) ? 0 : 255;
			bytes[(y * size + x) * 3] = value;
			bytes[(y * size + x) * 3 + 1] = value;
			bytes[(y * size + x) * 3 + 2] = value;
		}
	}

	return wxImage(wxSize(size, size), bytes);
}
