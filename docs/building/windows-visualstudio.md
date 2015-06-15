Building Astron with Visual Studio on Windows
=============================================
**Author(s)**

Dean Aug ~ (1/3/2015)

Michael "mwass" Wass ~ (6/15/2015)

1. Introduction
---------------
This document describes all steps required to build the *Astron* server suite on a windows machine; from acquisition of the *Astron* code and all code components it depends on (referred to as "dependencies"), to building of those dependencies and *Astron* itself. These are general instructions that can be followed by anyone using *Astron*.

This document covers building Astron in a Windows environment only.

2. Downloading Astron Source Code
---------------------------------

The Astron source code is open-source and available on GitHub. 

Prior to downloading the *Astron* code, create a folder where you will store the source code. For this document, we created a folder named `Compiling-Astron`.

In command prompt (run as Administrator to allow file updates), change to the directory one you just created in the previous step.

Execute the following command to clone the *Astron* repository in that directory: 

`git clone https://github.com/Astron/Astron.git`

This will create an *Astron* subdirectory, containing all the current files from the *Astron* repository.

3. Downloading Astron Dependencies
----------------------------------

*Astron* has dependencies on the following code components:

`yaml-cpp 0.5+`
`boost 1.55+`
`openssl 1.0.1+`

Each must be downloaded and installed in the *Astron* source tree created in the previous section.

These are the dependencies for Astron compiled for use with a YAML Database.

### 1. YAML-CPP
The available *yaml-cpp* downloads are listed on the following site:

`https://code.google.com/p/yaml-cpp/downloads/list`

From the site listed above, select the link at the top to download the latest version of *yaml-cpp* (version 0.5.1 at the time of this writing).

From the resulting page for that version, select the download link.

Download and/or open the version's zip file, and extract its contents to the following directory:

`Compiling-Astron\Astron\dependencies`

This will create a *yaml-cpp* subdirectory (named after the version), containing all the files for that version.

### 2. Boost

The available *boost* downloads are listed on the following site:

`http://www.boost.org/users/download/`

From the site listed above, select 'Prebuilt windows binaries' under Other Downloads to download the latest version of *boost* binaries (version 1.57.0 at the time of this writing).

From the resulting page, select the link for the latest version (1.57.0 at the time of this writing).

From the resulting page, select the link for *Visual Studio 12 2013* Win64 (boost_1_57_0-msvc-12.0-64.exe at the time of this writing).

Save the setup file when prompted.

Run the resulting setup file, with the default directory, which at the time of this writing is:

`C:\local\boost_1_57_0`

Move the resulting setup folder (under the local folder, so `boost_1_57_0` at the time of this writing) to the following directory:

`Compiling-Astron\Astron\dependencies`

This will create a *boost* subdirectory (named after the version), containing all the files for that version.

### 3. OpenSSL

The available *OpenSSL* downloads are listed on the following site:

`http://www.npcglib.org/~stathis/blog/precompiled-openssl`

From the site listed above, select the link for the latest version's Visual 2013 download (openssl-1.0.1j-vs2013.7z at the time of this writing).

Save the file when prompted, and extract its contents to the following directory:

`Compiling-Astron\Astron\dependencies`

After all files are extracted, delete the following directories (these contain the 32-bit library versions, but ***Astron* requires the 64-bit versions;** and if these directories are present *CMake* will detect them and erroneously try to use them when it prepares for the Astron build).

`Compiling-Astron\Astron\dependencies\openssl-1.0.1j\bin`

`Compiling-Astron\Astron\dependencies\openssl-1.0.1j\lib`


This will create a *OpenSSL* subdirectory (named after the version), containing all the files for that version. If this subdirectory name includes a suffix such as -vs2013, remove that from the name.

Now that all the dependencies are in place, the Astron\dependencies directory should have a structure similar to the following:

![icon](http://i.imgur.com/dGe4r8d.png)

### 4. Building Astron Dependencies

Each of the non-binary dependencies downloaded must also be built, before they can be used as part of the *Astron* build.

Prior to building the *Astron* code, ensure the folder shown below is present. The directory shown below is not a required name and/or location for the build folder. The name and/or directory location are decided by the user.

`Compiling-Astron\AstronBuild`

Some of the dependencies require the use of *CMake GUI* to build, if required the latest installer version of this can be downloaded from `http://www.cmake.org/download/` (cmake-3.1.0-win32-x86.exe at the time of this writing).

### 1. YAML-CPP

This is built by first using the *CMake GUI* to generate all files required for building in VS2013, then using those files to perform the build in VS013.

Configure the following directories for source code and binary locations:

`Compiling-Astron\Astron\dependencies\yaml-cpp-0.5.1`
`Compiling-Astron\Astron\dependencies\yaml-cpp-0.5.1`

Add the following cache entry (or equivalent if different Boost version is used):

`Name: BOOST_ROOT`
`Type: PATH`
`Value: Compiling-Astron\Astron\dependencies\boost_1_57`

Select `Configure`, then *Visual Studio 12 2013* Win64 for the project generator. (NOTE! Currently you must use VS2013 to compile Astron!)

Once *CMake*) has completed configuration, settings for *yaml-cpp* should result similar to the following:

![icon](http://i.imgur.com/ur2oZ8p.png)

Select `Generate` to generate the files needed to build *yaml-cpp* in VS2013.

Add  `#include <algorithm>` to the following file, to resolve use of std::max:

`Compiling-Astron\Astron\dependencies\yaml-cpp-0.5.1\src\ostream_wrapper.cpp`

Open the following solution in VS2013:

`Compiling-Astron\Astron\dependencies\YAML_CPP.sln`

Build all projects in the solution, the default build configuration is for `Debug`. Final output is:

`Compiling-Astron\Astron\dependencies \Debug\libyaml-cppmdd.lib`

In the solution Configuration Manager, change the Active solution configuration to `Release`, then build all projects in the solution again. Final output is:

`Compiling-Astron\Astron\dependencies \Release\libyaml-cppmd.lib`

The resulting `Debug` and `Release` yaml libraries are:

![icon](http://i.imgur.com/cnZ5BDi.png)

![icon](http://i.imgur.com/H1LL5O8.png)

### 2. Boost
Since the *boost* binaries were downloaded, no build is required.

The resulting *boost* libraries are:

![icon](http://i.imgur.com/o654DSU.png)

![icon](http://i.imgur.com/V4bRKzz.png)

### 3. OpenSSL 
Since the openssl binaries were downloaded, no build is required.

The resulting boost libraries are:

![icon](http://i.imgur.com/fO8X8li.png)

5. Building Astron Source Code
------------------------------

This is built by first using the *CMake GUI* to generate all files required for building in VS2013, then using those files to perform the build in VS013.

Configure the following directories for source code and binary locations:

Source: `Compiling-Astron\Astron`

Binary: `Compiling-Astron\AstronBuild`

Add the following cache entries (or equivalent if different versions used):

`Name: BOOST_ROOT`
`Type: PATH`
`Value: Compiling-Astron\Astron\dependencies\boost_1_57`

`Name: Boost_LIBRARY_DIR`
`Type: PATH`
`Value: Compiling-Astron\Astron\dependencies\boost_1_57\ lib64-msvc-12.0`

`Name: OPENSSL_ROOT_DIR`
`Type: PATH`
`Value: Compiling-Astron\Astron\dependencies\openssl-1.0.1j`

Select `Configure`, then *Visual Studio 12 2013* Win64 for the project generator.

Once* CMake* has completed configuration, settings for the dependencies should be similar to the following **(NOTE: Make sure the 64-bit paths are being used in the settings for groups Boost, LIB, and SSL; attempting to use 32-bit versions will result in link-time errors when building astron in Visual Studio):**

![icon](http://i.imgur.com/LdpTEwo.png)

Select `Generate` to generate the files needed to build `astrond` in VS2013.

Open the following solution in VS2013:

`Compiling-Astron\AstronBuild\Astron.sln`

Build all projects in the solution, the default build configuration is for `Debug`. Final output is:

`Compiling-Astron\AstronBuild\Debug\astrond.exe`

This is the final step in the *Astron* build, all required libraries/executables are now in place. The resulting *Astron* executable is:

![icon](http://i.imgur.com/Gme9ncx.png)

6. Running Astron
-----------------

In addition to the executable produced in the previous section, *Astron* needs 3 additional files to run. `astrond.yml`, `libeay32MDd.dll`, and `ssleay32MDd.dll`.

An example `astrond.yml` can be found in the *Astron* repository [here](docs/config/astron.example.yml).

`libeay32MDd.dll` and `ssleay32MDd.dll` can both be found in the *OpenSSL* folder.

`key.pem` and `cert.pem` are files that can be used for TLS/SSL configuration, which is not required for use.

![icon](http://i.imgur.com/Q5xUPww.png)

To run *Astron*, navigate to the directory with the executable and supporting files in a command prompt, and execute the `astrond` command. Once Astron is running, it will produce messages similar to the following:

![icon](http://i.imgur.com/NQscEUI.png)

**NOTE:** The `astrond.exe` running in this picture was the `Debug` EXE. The `Release` EXE will produce slightly different messages.
