# Nexora - Windows server

## Requirments & Dependencies

- Visual Studio 2022 with Dekstop C++ development package installed
- vcpkg with the following dependencies installed:
    - **ASSO**: ```vcpkg install asio```
    - **wxWidgets**: ```vcpkg install wxwidgets```

## Build

Before building Nexora [softcam](https://github.com/tshino/softcam) needs to be built.

### Softcam library

Open ```3rdparty/softcam/softcam.sln``` and build the solution in ```Release x64``` configuration.

Next open ```3rdpart/softcam/examples/softcam_installer.sln``` and build the solution in ```Release x64``` configuration.

For more information about the building process of softcam see [this](https://github.com/tshino/softcam?tab=readme-ov-file#how-to-build-the-library).

### Nexora

Open the ```Nexora.sln``` and build the solution in ```Release x64``` configuration. All required files will be placed in the ```dist``` directory.

Now from the root directory you can run ```install.bat``` to install the DirectShow filter (softcam.dll)

