A command line client and C++ library that uses the Autolab REST API.

## Build

### Dependencies

This C++ project has the following dependencies:

- [openssl](https://www.openssl.org/): For crypto operations
- [libcurl](https://curl.haxx.se/libcurl/): For HTTP operations
- [rapidjson](https://github.com/Tencent/rapidjson): For JSON processing

CMake is already setup to automatically handle acquiring and setting up rapidjson.

Please make sure openssl and libcurl libraries are installed prior to building. They can usually be installed with the system's package manager on Linux.

For example, on Ubuntu, users can install these dependencies by running:  
`sudo apt-get install libssl-dev libcurl4-openssl-dev`

### Getting Client Credentials

The program requires client credentials in order to build.

First, register a new application on your deployment of Autolab. The redirect_uri should be '\<host\>/device_flow_auth_cb' since this application uses the 'device_flow' authorization method. The scopes should be `user_info user_courses user_scores user_submit`. To test building without credentials, use empty strings as credentials and continue.

Then, create file src/app_credentials.h by making a copy of src/app_credentials.h.template, and enter the generated client_id and client_secret into the predefined fields in the file.

### Build Instructions

This project uses CMake. On Linux, it generates Makefiles for the project, which can then be used by the `make` command to perform regular incremental builds.

#### Quick Build & Install Script for Bash Users

We've written an install script that builds the entire project, installs the binary to your system, and installs the bash autocompletion script. You can run it by executing `./install/install.sh`. It needs sudo access in order to copy files to protected directories (details below)

#### Manual Build and Install

1. create 'build' directory under project root directory.
2. cd into 'build', run `cmake ..`.
3. inside 'build', run `make`.

This will build two targets:
1. An executable build/src/autolab
2. A static library build/lib/autolab/libautolab.a

You can optionally run `sudo make install` to install the built binaries (typically to `/usr/local/bin/`).

##### Autocompletion ( bash users only :( )

After installing manually, users can cd out of build and execute the following commands:

1. `sudo cp autocomplete/autolab /etc/bash_completion.d/`
2. `. /etc/bash_completion.d/autolab`

This will move our autocompletion script out of a local folder and into the bash autocompletion directory. To learn more about bash autocompletion, see https://debian-administration.org/article/317/An_introduction_to_bash_completion_part_2

### Build Options

#### Release vs Debug

There are two kinds of builds available: release and non-release. Release builds do not contain debug output (output that use Logger::debug).

The default is non-release builds. To build a release version, when inside the 'build' directory, run `cmake -Drelease=ON ..` (note the periods at the end), then run `make`.

#### Build Variant

In addition to specifying the version number in CMakeLists.txt, a 'build variant' string can be used to include metadata about the build. This info will be shown when running `autolab --version`.

For example, in our official build for the CMU shark machines, we run cmake with `-Dvariant=cmu-shark`. This helps indicate what the executable was built for.

## How to use

### Using the command line client

Run 'autolab -h' to find out the commands available.

### Using the library

To use the autolab client library in your own C++ program, include the header files in include/autolab/, then link against libautolab.a. Make sure you are compiling with at least C++11.
