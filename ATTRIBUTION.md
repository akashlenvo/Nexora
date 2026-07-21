# Attribution and project history

Nexora is a renamed and substantially redesigned continuation of the MIT-licensed [VCamdroid](https://github.com/darusc/VCamdroid) project by darusc.

The original copyright and MIT permission notice remain in `LICENSE`, as required by that license. Copyright for Nexora-specific modifications is held by Yves Godoy. This does not transfer ownership of the original author's work.

## Why this fork exists

The upstream v1.3.1 release paired a newer Windows client with an older Android APK. The APK sent only the raw device model (for example `SM-S928U`), while Windows expected a length-prefixed device descriptor. The first two ASCII bytes were interpreted as a 21,325-byte string length and an unchecked `memcpy` caused access violation `0xc0000005` in `VCRUNTIME140.dll`.

Nexora fixes that release mismatch by shipping the current Android protocol implementation and adds bounds-checked deserialization plus segmented TCP-handshake handling on Windows. Malformed and legacy clients are rejected safely.

## Third-party code

The `windows/3rdparty/softcam` directory is a Git submodule of Toshihiko Shimizu's softcam project and retains its own license and notices. Dependencies downloaded by Gradle, vcpkg or GitHub Actions retain their respective licenses.
