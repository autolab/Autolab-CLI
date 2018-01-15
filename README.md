A command line autolab client that uses the Autolab REST API.

## Build

### Dependencies

This C++ project has the following dependencies:

- [libcurl](https://curl.haxx.se/libcurl/): For HTTP operations
- [rapidjson](https://github.com/Tencent/rapidjson): For JSON processing

Please make sure the dependencies are installed prior to building.

### Build Instructions

This project uses CMake. To build:

1. create 'build' directory under project root directory.
2. cd into '/build', run `cmake ..`.
3. inside '/build', run `make`.

The built binary is /build/src/autolab.

You can optionally run `make install` to install the built binaries (typically to `/usr/local/bin/`).

### Build Options

There are two builds available: release and non-release. Release builds do not contain debug output (output that uses Logger::debug).

The default is non-release builds. To build a release version, when inside 'build' directory, run `cmake -Dreleaese=ON .` (note the period at the end), then run `make`.

## How to use

First, register a new application on your deployment of Autolab. The redirect_uri should be '\<host\>/device_flow_auth_cb' since this application uses the 'device_flow' authorization method. Enter the generated client_id and client_secret in the source code. (Right now, this is done by modifying the constants defined on the top of 'src/main.c').

Run 'autolab -h' to find out the commands available.
