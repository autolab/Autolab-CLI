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

First, register a new application on your deployment of Autolab. Go to "Manage Autolab" and click on "Manage API Applications". Then, click on "New Application".

![Screen Shot 2023-02-12 at 14 49 56](https://user-images.githubusercontent.com/25730111/218333728-fba04ebb-fea2-437b-abad-4aaae91c9794.png)

Then in "New Application", specify a name for the application, and the `redirect_uri`. The `redirect_uri` should be `<host>/device_flow_auth_cb` since this application uses the `device_flow` authorization method, where `<host>` is the your Autolab domain. For example, for Nightly, use `https://nightly.autolabproject.com/device_flow_auth_cb`.

The scopes should be `user_info user_courses user_scores user_submit`. To test building without credentials, use empty strings as credentials and continue.

![Screen Shot 2023-02-12 at 14 55 39](https://user-images.githubusercontent.com/25730111/218333852-f739cc46-bcb7-44d6-9209-6b049bfbb31c.png)

Then, create file `lib/all_servers_dirname.h` by making a copy of `lib/all_servers_dirname.h.template`, decide where you want to store info text files
for each Autolab server and enter the absolute path to that directory into the
`all_servers_dirname` field in the file.

Inside of this directory that you created, insert files that follow the format as can be seen in
`lib/server_info_example.json`. While this can be populated manually, you can
also go back to Manage API Applications and click "Download Config" to download
the file. A daemon can populate the courses field with course names according
to the Autolab server. Make sure that there are no duplicate courses or server 
names (the automatic config sets this to be equal to the URL). The name of the files do not matter. 

You should then after building autolab-cli, be able to run `autolab setup`, and successfully authorize the CLI with your Autolab deployment.

### Build Instructions

This project uses CMake. On Linux, it generates Makefiles for the project, which can then be used by the `make` command to perform regular incremental builds.

#### Quick Build & Install Script for Bash Users

We've written helper scripts to build and install the project. These are located in `scripts/`. All of them need to be run from the repo root.

To build the project using CMake, run `scripts/build.sh`.

To install the autolab binary to a particular location, run `scripts/install.sh [DIRNAME]`. If no location is supplied, a default will be used, usually `/usr/local/bin`. This may prompt for a sudo password if the location is protected.

#### Manual Build and Install

1. create 'build' directory under project root directory.
2. cd into 'build', run `cmake ..`.
3. inside 'build', run `make`.

This will build two targets:
1. An executable build/src/autolab
2. A static library build/lib/autolab/libautolab.a

You can optionally run `sudo make install` to install the built binaries (typically to `/usr/local/bin/`).

##### Autocompletion ( bash users only :( )

From the repo root, bash users can run `sudo ./scripts/bash_autocomplete.sh`, which will enable autocompletion.

This will move our autocompletion script out of a local folder and into the bash autocompletion directory. To learn more about bash autocompletion, see https://debian-administration.org/article/317/An_introduction_to_bash_completion_part_2

### Build Options

#### Release vs Debug

There are two kinds of builds available: release and debug. Release builds do not contain debug output (output that use `Logger::debug`).

The default is debug builds. To build a release version, when inside the 'build' directory, run `cmake -DCMAKE_BUILD_TYPE=Release ..` (note the periods at the end), then run `make`.
Alternatively (but less preferable), you can use the flag `-Drelease=ON`.

#### Build Variant

In addition to specifying the version number in CMakeLists.txt, a 'build variant' string can be used to include metadata about the build. This info will be shown when running `autolab --version`.

For example, in our official build for the CMU shark machines, we run cmake with `-Dvariant=cmu-shark`. This helps indicate what the executable was built for.

## How to use

### Using the command line client

Run 'autolab -h' to find out the commands available.

### Using the library

To use the autolab client library in your own C++ program, include the header files in include/autolab/, then link against libautolab.a. Make sure you are compiling with at least C++11.
