libanim
=======

About
-----
Libanim is a C animation library, which can be used as a generic framework to
add keyframe interpolation tracks for any single-valued or 3-vector parameters,
or at a slightly higher level as a hierarchical PRS (position/rotation/scaling)
animation node framework for 3D graphics programs.

Version 2 of libanim dropped the dependency to libvmath, and instead carries a
copy of gph-cmath (https://github.com/jtsiomb/gph-cmath) internally. The API
has been reworked to avoid forcing a dependency to any math library to the user
program, relying on floats and float pointers instead, which can be aliased to
any kind contiguous `x,y,z` vector and `x,y,z,w` quaternion, or simple arrays
of floats. Matrix arguments are expected to be arrays of 16 contiguous floats,
in OpenGL-compatible order.

Programs written for earlier versions of libanim, and using the high-level PRS
interface in `anim.h` are not source-compatible, nor binary-compatible with
libanim 2. Though in practice the API changes are minor, and porting should be
straightforward. Programs using only the low-level keyframe tracks in `track.h`
are unaffected by these changes.

License
-------
Copyright (C) 2012-2018 John Tsiombikas <nuclear@member.fsf.org>

This program is free software. You may use, modify, and redistribute it under
the terms of the GNU Lesser General Public License v3 or (at your option), any
later version published by the Free Software Foundation. See COPYING and
COPYING.LESSER for details.

Build
-----
To build and install libanim on UNIX, run the usual:

    ./configure
    make
    make install

See `./configure --help` for a complete list of build-time options.

To cross-compile for windows with mingw-w64, try the following incantation:

    ./configure --prefix=/usr/i686-w64-mingw32
    make CC=i686-w64-mingw32-gcc AR=i686-w64-mingw32-ar sys=mingw
    make install sys=mingw
