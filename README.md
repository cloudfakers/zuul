Zuul: The Abiquo Gatekeeper
===========================

Zuul is a small IRC bot used to open the office door. It runs on a Raspberry Pi
and sends a signal to the appropriate GPIO pin when the "open door" command is
issued. This way wa can keep working on our shit without having to traverse the
office everytime someone rings.

Prerequisites
-------------

The IRC bot is implemented using the [Circus](https://github.com/nacx/circus) framework. You can download and install it followign the instructions in its README.

It also uses [WiringPi](http://wiringpi.com/download-and-install/). You can also download and install it following the instructions on its site.

Compiling
---------

To compile Zuul, you can simply run:

    make

If you want to install it in your system, you can do it as usual. It will be installed by default in
/usr/local, but you can customize the installation directory by adding the *PREFIX* parameter:

    make install                       # Installs Zuul in /usr/local
    make install PREFIX=<install dir>  # Installs Zuul in <install dir>

Running
-------

You can run Zuul as follows, to let the bot join the specified channel. If the channel is password
protected, you can also provide the password as an additional argumen.

    ./zuul <irc server> <irc port> <nickname> <channel> [<channel password>]

License
-------

This sowftare is licensed under the Apache License 2.0. See LICENSE file for details.
