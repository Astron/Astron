Building with GNU _Make_ on Linux
-----------------------------------

### Dependencies ###
You will need to, of-course, clone the repository.
After you have the repository you will need to download and build Astron's
dependencies.  See the [building readme](build-readme.md) for instructions.


### Compiling ###
After setting up your environment you can compile with any of the following:

**For release:**

    cmake -DCMAKE_BUILD_TYPE=Release . && make

**For development (with Trace and Debug messages):**

	cmake -DCMAKE_BUILD_TYPE=Debug . && make

**For development (without Trace and Debug messages):**

	cmake . && make


### Handling Build Artifacts ###
When contributing to the Astron repository or a branch you should not add build
artifacts to your commit.  Because build artifacts vary wildly between all the
operating systems and compilers that Astron will compile with, they are not
included in the `.gitignore` file.

Instead, edit the `.git/info/exclude` file for your local clone.

The following should be added for GNU Make on Linux:  
```
## Compilation Artifacts ##
astrond
*.[oa]
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
Makefile
```
