SYCL Parallel STL
==============================

This project features an implementation of the Parallel STL library
using the Khronos SYCL standard.

What is Parallel STL
-----------------------

Parallel STL is an implementation of the Technical Specification for C++
Extensions for Parallelism, current document number
[N4507](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4507.pdf).
This technical specification describes _a set of requirements for
implementations of an interface that computer programs written in
C++ programming language may use to invoke algorithms with parallel
execution_.
In practice, this specification aimed at the next C++ standard,
offers the opportunity to users to specify _execution policies_ to
traditional STL algorithms, which will enable the execution of
those algorithms in parallel.
The various policies can specify different kinds of parallel execution.
For example,

    std::vector<int> v = ...
    // Traditional sequential sort
    std::sort(vec.begin(), vec.end());
    // Explicit sequential sort
    std::sort(seq, vec.begin(), vec.end());
    // Explicit parallel sort
    std::sort(par, vec.begin(), vec.end());


What is SYCL
----------------------

[SYCL](https://www.khronos.org/opencl/sycl) is a royalty-free,
cross-platform C++ abstraction layer that builds on top of OpenCL.
SYCL enables single-source development of OpenCL applications in C++ whilst
enabling traditional host compilers to produce standard C++ code.

The SyclSTL
---------------------

SyclSTL exposes a SYCL policy on the experimental::parallel namespace
that can be passed to standard STL algorithms for them to run on SYCL.
Currently, the following STL algorithms are implemented:

* sort : Bitonic sort for ranges which size is power of two, sequential sort
otherwise.
* transform : Parallel iteration (one thread per element) on the device.
* for\_each  : Parallel iteration (one thread per element) on the device.
* for\_each\_n : Parallel iteration (one work-item per element) on the device.
* count\_if : Parallel iteration (one work-item per 2 elements) on device.
* reduce : Parallel iteration (one work-item per 2 elements) on device.
* inner\_product: Parallel iteration (one work-item per 2 elements) on device.
* transform\_reduce : Parallel iteration (one work-item per 2 elements) on device.

Some optimizations are implemented, for example, the ability of passing
iterators to buffers rather than STL containers to reduce the amount of
information copied in and out, and the ability of specifying a queue
to the SYCL policy so that queue is used for the various kernels (potentially
enabling asynchronous execution of the calls).

Building the project
----------------------


The project uses CMake 3.0.2 in order to produce build files. 
More recent versions may work. 
In Linux, simply create a build directory and run CMake as follows:

    $ mkdir build
    $ cd build
    $ cmake ../ -DSYCL_PATH=/path/to/sycl \
    $ make

For Windows platforms, create a build directory and run CMake as follows:

    $ cmake ..\ -DSYCL_PATH=c:\Path\To\Your\SYCL

Usual CMake options are available (e.g. building debug or release). 
Makefile and Ninja generators are supported on Linux.
Visual Studio and Ninja generators are supported on Windows, for example
to build with the Visual Studio generator, you can:

    $ cmake ..\  -DSYCL_PATH=c:\Path\To\Your\SYCL -G "Visual Studio 12 Win64"

If Google Mock is found in external/gmock, a set of unit tests is built.
Unit tests can be run by running Ctest in the binary directory.

To enable building the benchmarks, enable the *PARALLEL_STL_BENCHMARKS* option
in the cmake configuration line.

When building with a SYCL implementation that has no device compiler,
enable the *SYCL_NO_DEVICE_COMPILER* option to disable the specific 
CMake rules for intermediate file generation.

Refer to your SYCL implementation documentation for 
implementation-specific building options.

Building the documentation
----------------------------

Source code is documented using Doxygen.
To build the documentation as an HTML file, navigate to the doc
directory and run doxygen from there.

    $ cd doc
    $ doxygen

This will generate the html pages inside the doc\_output directory.

Limitations
------------

* The Lambda functions that you can pass to the algorithms have the same
restrictions as any SYCL kernel. See the SYCL specification for details
on the limitations.

* While using lambda functions, the compiler needs to find a name for that lambda
function. To provide a lambda name, the user has to do the following:

    cl::sycl::queue q;
    sycl::sycl_execution_policy<class SortAlgorithm3> snp(q);
    sort(snp, v.begin(), v.end(), [=](int a, int b) { return a >= b; });

* Be aware that some algorithms may run sequential versions if the number of
elements to be computed are not power of two. The following algorithms have
this limitation: sort, inner_product, reduce, count_if and transform_reduce.

* Refer to SYCL implementation documentation for implementation-specific 
building options.

Copyright and Trademarks
------------------------

Intel and the Intel logo are trademarks of Intel Inc. AMD, the AMD Arrow
logo, and combinations thereof are trademarks of Advanced Micro Devices, Inc.
OpenCL and the OpenCL logo are trademarks of Apple Inc. used by permission by
Khronos. Other names are for informational purposes only and may be trademarks
of their respective owners.
