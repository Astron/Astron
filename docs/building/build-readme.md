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
 * boost 1.55+
 * openssl 1.0.1+

Get the boost library from http://www.boost.org/users/download/, or install from a package on linux.  Very old versions of the boost library may not work as there are dependencies on some features of boost::asio and boost::icl.
In some environments, you may have to set the BOOST_ROOT or BOOSTROOT environment variable to the root of your compiled boost libraries.

You can download the libyaml-cpp dependency directly from https://yaml-cpp.googlecode.com/files/yaml-cpp-0.5.1.tar.gz.  This dependency should typically be compiled and placed directly underneath the Astron/dependencies folder.

Now that you have the dependencies you can build!

We use CMAKE to handle cross-platform compiling.
See the individual guides for the compiler you intend on using:

 - Linux/Make : [instructions](linux-gnu-make.md)
 - Windows/VisualStudio : [instructions](windows-visualstudio.md)
