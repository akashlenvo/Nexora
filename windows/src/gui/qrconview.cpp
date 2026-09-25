#include "gui/qrconview.h"
#include <asio.hpp>

QrconView::QrconView(std::string name, std::string address, std::string port, wxSize displaySize) : wxDialog(nullptr, wxID_ANY, "Connect with QR code")
{
	auto sizer = new wxBoxSizer(wxVERTICAL);
	auto wrapper = new wxBoxSizer(wxVERTICAL);

	auto qrcode = GenerateQRCode(address + ":" + port);
	auto image = GenerateImageFromQR(qrcode);

	wxStaticBitmap* canvas = new wxStaticBitmap(this, wxID_ANY,
		wxBitmap(image.Scale(displaySize.GetWidth(), displaySize.GetHeight())));

	wrapper->Add(new wxStaticText(this, wxID_ANY, "Scan the QR to connect automatically"), 0, wxALIGN_CENTER | wxTOP, FromDIP(30));
	wrapper->Add(canvas, 0, wxALIGN_CENTER | wxALL, FromDIP(15));
	wrapper->Add(new wxStaticText(this, wxID_ANY, wxString::FromUTF8(name)), 0, wxALIGN_CENTER | wxBOTTOM, FromDIP(2));
	auto addressLabel = new wxStaticText(this, wxID_ANY, "Address: " + wxString::FromUTF8(address));
	wrapper->Add(addressLabel, 0, wxALIGN_CENTER);
	wrapper->Add(new wxStaticText(this, wxID_ANY, "Port: " + wxString::FromUTF8(port)), 0, wxALIGN_CENTER | wxBOTTOM, FromDIP(30));
	wrapper->Add(new wxStaticText(this, wxID_ANY, "Wrong network? Enter this computer's IPv4 address:"), 0, wxALIGN_CENTER | wxBOTTOM, FromDIP(5));
	auto addressInput = new wxTextCtrl(this, wxID_ANY, wxString::FromUTF8(address));
	wrapper->Add(addressInput, 0, wxEXPAND | wxBOTTOM, FromDIP(8));
	auto updateButton = new wxButton(this, wxID_ANY, "Update QR");
	wrapper->Add(updateButton, 0, wxALIGN_CENTER | wxBOTTOM, FromDIP(20));
	updateButton->Bind(wxEVT_BUTTON, [this, addressInput, addressLabel, canvas, port, displaySize](wxCommandEvent&) {
		const auto updatedAddress = addressInput->GetValue().ToStdString();
		asio::error_code error;
		const auto ip = asio::ip::make_address_v4(updatedAddress, error);
		if (error || ip.is_unspecified() || ip.is_loopback()) {
			wxMessageBox("Enter a valid IPv4 address for this computer's local network.", "Invalid address", wxOK | wxICON_WARNING, this);
			return;
		}
		const auto updatedImage = GenerateImageFromQR(GenerateQRCode(updatedAddress + ":" + port));
		canvas->SetBitmap(wxBitmap(updatedImage.Scale(displaySize.GetWidth(), displaySize.GetHeight())));
		addressLabel->SetLabel("Address: " + wxString::FromUTF8(updatedAddress));
		Layout();
	});

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
