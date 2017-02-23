Zuul: The Abiquo Gatekeeper
===========================

Zuul is a small IRC bot to control the door so we can just keep
doing our shit when someone rings.

The C code to access the GPIO registers is inspired by:
http://elinux.org/RPi_GPIO_Code_Samples

Prerequisites
-------------

The IRC bot is implemented using [Circus](https://github.com/nacx/circus). It needs to
be compiled and installed before building Zuul.

Compiling
---------

To compile Zuul, you can simply run:

    make

If you want to install it in your system, you can do it as usual. It will be installed by default in
/usr/local, but you can customize the installation directory by adding the *PREFIX* parameter:

    make install                       # Installs Zuul in /usr/local
    make install PREFIX=<install dir>  # Installs Zuul in <install dir>

License
-------

This sowftare is licensed under the Apache License 2.0. See LICENSE file for details.
