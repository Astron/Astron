How to Build Astron
------------------------

### Getting a copy of the code ###
Astron is available on GitHub through Git or Subversion cloning.

On your machine clone with:

    git clone https://github.com/Astron/Astron.git

**OR**

    svn checkout https://github.com/Astron/Astron



### Dependencies ###

 * libyaml-cpp 0.5+
 * libuv 1.23.0+
 * boost 1.55+ (optional if libyaml-cpp 0.6+)
 * openssl 1.0.1+

Get the boost library from http://www.boost.org/users/download/, or install from a package on linux.  Very old versions of the boost library may not work as there are dependencies on some features of boost::icl.
In some environments, you may have to set the BOOST_ROOT or BOOSTROOT environment variable to the root of your compiled boost libraries.

**Note:** libyaml-cpp version 0.6.0 and above *no longer* requires the boost dependency.

You can get libyaml-cpp from https://github.com/jbeder/yaml-cpp/. This dependency should typically be compiled and placed directly underneath the Astron/dependencies folder. Alternatively, you can install it from a package.

Grab the libuv dependency from https://github.com/libuv/libuv or install from a package on linux.

Now that you have the dependencies you can build!

We use CMAKE to handle cross-platform compiling.
See the individual guides for the compiler you intend on using:

 - Linux/Make : [instructions](linux-gnu-make.md)
 - Windows/VisualStudio : [instructions](windows-visualstudio.md)
