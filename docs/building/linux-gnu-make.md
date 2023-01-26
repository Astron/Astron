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
