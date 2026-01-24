<h1 align="center">
  <sub>
    <img src="imgs/icon2.png" width="150">
  </sub>
  <br>
  VCamdroid
</h1>

<p align="center">Turn your Android phone into a high-performance Windows webcam.</p>

<p align="center">
  <a href="https://github.com/darusc/VCamdroid/blob/main/LICENSE">
    <img src="https://img.shields.io/github/license/darusc/VCamdroid?style=for-the-badge" alt="License">
  </a>
  <a href="https://github.com/darusc/VCamdroid/releases">
    <img src="https://img.shields.io/github/v/release/darusc/VCamdroid?style=for-the-badge" alt="Release">
  </a>
  <a href="https://github.com/darusc/VCamdroid/releases">
    <img src="https://img.shields.io/github/downloads/darusc/VCamdroid/total?style=for-the-badge" alt="Downloads">
  </a>
</p>



## Description

VCamdroid allows you to seamlessly use your mobile device’s camera as a virtual webcam on your PC. Built using a custom DirectShow filter and the [Softcam library](https://github.com/tshino/softcam), it ensures compatibility with popular applications like Zoom, OBS, Discord, and Teams. Whether wired (via ADB) or wireless (via Wi-Fi), VCamdroid delivers a low-latency, hardware-accelerated video feed directly to your desktop.

<p align="center">
  <img src="imgs/demo.gif" width="600" alt="VCamdroid Demo">
</p>

## Key Features

* **Universal Compatibility:** Works with any Windows application that supports standard webcams.
* **Flexible Connectivity:** Supports high-speed wired connections (via ADB) and convenient wireless connections (via Wi-Fi).
* **Multi-Device Support:** Connect multiple Android devices simultaneously and switch between them instantly.
* **Full Camera Control:** Remotely toggle between front and back cameras, adjust resolutions, and enable flash.
* **Image Adjustments:** Real-time controls for rotation, mirroring (flip), brightness, contrast, and saturation.
* **Zero-Config Pairing:** Automatically connects over USB via ADB; straightforward QR code pairing for Wi-Fi.

---

## Installation Guide

### Prerequisites
* **PC:** Windows 10 or 11.
* **Phone:** Android 7.0 (Nougat) or higher.

### Step 1: Install on Windows
1.  Download the latest binaries from the [**Releases Page**](https://github.com/darusc/VCamdroid/releases).
2.  Extract the ZIP archive.
3.  Right-click `install.bat` and select **Run as Administrator**.
    * *Note: This script registers `softcam.dll` with the system, making the virtual webcam device visible to other applications.*

### Step 2: Install on Android
1.  Connect your phone to your PC via USB.
2.  Ensure **USB Debugging** is enabled (see instructions below).
3.  Run `install_apk.bat` on your PC to automatically install the app on your phone.
    * *Alternatively, transfer the APK file to your phone and install it manually.*

### 💡 How to Enable USB Debugging
1.  Go to **Settings > About Phone**.
2.  Find **Build Number** and tap it **7 times** until you see "You are now a developer!"
3.  Go back to **Settings > System > Developer Options**.
4.  Toggle **USB Debugging** to **ON**.
    * *For device-specific steps, refer to the [official Android documentation](https://developer.android.com/studio/debug/dev-options).*


## Usage Instructions

### Wired Connection (USB / ADB)
*Recommended for lowest latency and highest stability.*

1.  Connect your phone to the PC via USB.
2.  Launch the **VCamdroid Desktop Client**.
3.  Launch the **VCamdroid App** on your phone.
4.  The connection is automatic. The app status should change to "Connected."

### Wireless Connection (Wi-Fi)
1.  Ensure both your PC and phone are on the same Wi-Fi network.
2.  Launch the **VCamdroid Desktop Client** and select the **Connect** tab to reveal a QR Code.
3.  Launch the **VCamdroid App** on your phone.
4.  Tap the **Scan QR Code** button and point your camera at the PC screen.


## Technical Architecture

### Networking Protocol
VCamdroid utilizes the industry-standard **RTSP (Real-Time Streaming Protocol)** to ensure robust, low-latency video transmission between the Android device and the Windows client.

1.  **Transport Layer:**
    * **Wi-Fi Connection:** The Windows client connects directly to the RTSP server running on the Android device over the local network.
    * **USB Connection:** To enable wired communication, the application uses **ADB Port Forwarding**. Since ADB only supports TCP forwarding, the RTSP stream is tunneled exclusively over **TCP** (interleaved RTSP). This creates a stable, high-bandwidth tunnel via `localhost` that bypasses network interference.

2.  **Stream Handling Libraries:**
    * **Server Side (Android):** Powered by the [RootEncoder](https://github.com/pedroSG94/RootEncoder) library. It handles the complex tasks of interfacing with the Android encoder, packetizing the video data into RTP packets, and managing the RTSP server session.
    * **Client Side (Windows):** Utilizes [FFmpeg](https://ffmpeg.org/), the leading multimedia framework, to robustly demux the RTSP stream and decode the incoming video packets.

<p align="center"><img src="imgs/network.png" width="60%"></p>

### Video Pipeline
The pipeline is engineered for performance, offloading image processing to the Android GPU before compression to minimize latency and bandwidth.

1.  **Capture, Process & Encode (Android):**
    * **Capture:** Video frames are captured using the modern **Camera2 API**.
    * **Pre-Processing:** Raw frames are processed on the GPU using OpenGL. Operations like **Rotation**, **Mirroring (Flip)**, and **Color Correction** are applied here *before* encoding, ensuring the stream is "ready-to-display."
    * **Hardware Encoding:** The processed frames are passed to the device's hardware **MediaCodec** (supporting **H.264** or **H.265/HEVC**). This offloads compression from the CPU.
    * **Transmission:** **RootEncoder** encapsulates the encoded stream into RTP packets and transmits them over the active network connection.

2.  **Decode & Render (Windows):**
    * **Decoding:** The Windows client receives the RTSP stream and uses **FFmpeg** to decode the compressed H.264/H.265 frames into raw image data (YUV/RGB).
    * **Output:**
        * **UI Preview:** The decoded frame is rendered immediately to the application window for live monitoring.
        * **Virtual Device:** The frame is written to a ring buffer managed by the [Softcam](https://github.com/tshino/softcam) library. Softcam acts as the bridge between the user application and the system-registered DirectShow filter, allowing third-party apps (Zoom, Teams, OBS) to treat the stream as a physical webcam device.

<p align="center"><img src="imgs/pipeline.png" width="50%"></p>


## Contributing

Contributions are welcome! Whether you want to fix a bug, improve performance, or add a new feature, feel free to fork the repository and submit a Pull Request.

### Development Setup

**📱 Android App:**
* Open the `android/` folder in **Android Studio**.
* Sync with Gradle and build.
* **Key Libraries:** Uses [RootEncoder](https://github.com/pedroSG94/RootEncoder) for RTSP streaming and Camera2 APIs.

**💻 Windows Client:**
* Open the `windows/` folder in **Visual Studio**.
* **Dependencies:**
    * **wxWidgets:** For the GUI (install via vcpkg or source).
    * **FFmpeg:** For video decoding.
* **Build:** Open `VCamdroid.sln` and build for **x64 / Release**.
* **DirectShow Filter:** The virtual camera driver leverages the [Softcam](https://github.com/tshino/softcam) library.



        
