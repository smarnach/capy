Capy
====

Capy is a tool to create Python bindings for C++ classes and
functions.  Its most important feature is that it is a lightweight,
header-only library with no other dependencies than Python itself.
This makes it easy to include Capy in other projects.  The initial
intended purpose was the high-level wrapping of scientific simulations
in C++.  It isn't anywhere close as flexible as full-fledged libraries
like [Boost.Python][1] or [PyCXX][2].  (PyCXX was almost what I
wanted.  The main reason I decided against using it was that it's not
a header-only library.)

Features:

  * Hiding of reference counting and Python interface calls.  A useful
    subset of the Python interface has been wrapped, including classes
    to wrap Python dictionaries, lists and NumPy arrays.

  * Exceptions in Python API calls will be translated to to C++
    exceptions, making error checking very easy.

  * Exceptions in the C++ code will be caught and translated to Python
    exceptions, so you can catch them in Python code.  This will also
    catch uncaught exceptions from the Python API, which will be
    translated back to the original Python exception.

  * Evaluation of strings containing Python code in the current Python
    frame, giving access to the current global and local variables.

  * A simple interactive debugging console that can be started at any
    point in your C++ code.

What Capy is not:

  * A general-purpose wrapping tool.  The prototypes of functions and
    classes that can be wrapped are wuite restricted, so you can't use
    it to wrap a C++ library unchanged.  You rather need to design C++
    classes specifically to be wrapped by Capy.

  * Stable.

  * Documented.

In the unlikely case you think this might be useful for you, mail me
at sven (a) marnach (dot) net.  Maybe I'll write some documentation.

[1]: http://www.boost.org/doc/libs/release/libs/python/
[2]: http://cxx.sourceforge.net/


Licence
-------

Copyright (C) 2011-12 Sven Marnach

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


Linking against OpenMPI 1.3
---------------------------

Linking Python C extensions against OpenMPI 1.3 can be problematic.
Here are two solutions:

 * [Solution 1:][3]

        import dl
        import sys
        flags = sys.getdlopenflags()
        sys.setdlopenflags(flags | dl.RTLD_GLOBAL) 

 * [Solution 2:][4]

    Build your own OpenMPI package with the configure option
    `--enable-mca-static`.

[3]: http://www.open-mpi.org/community/lists/users/2009/02/8194.php
[4]: http://www.open-mpi.org/community/lists/users/2009/03/8582.php
