A command line autolab client that uses the Autolab REST API.

## Build

This project uses CMake. To build:

1. create 'build' directory under project root directory.
2. cd into '/build', run 'cmake ..'.
3. inside '/build', run 'make'.

The built binary is /build/src/autolab.

### Build Options

There are two builds available: release and non-release. Release builds do not contain debug output (output that uses Logger::debug).

The default is non-release builds. To build a release version, when inside 'build' directory, run 'cmake -Dreleaese=ON .', then run 'make'.

## How to use

Run 'autolab -h' to find out the commands available.