# ospi - INSTALL
## Overview

ospi is written in C++, in order to compile it, you will need a working toolchain and a c++ compiler.

## Dependencies

ospi depends on:
* PoDoFo <http://podofo.sourceforge.net/>
* Boost <http://www.boost.org/>

ospi can make use of:
* libcurl <http://curl.haxx.se/libcurl/>
* gmplib <http://gmplib.org/>

We use CMake <http://www.cmake.org/> to build and install ospi.

## Linux/Unix Installation

If you run a Debian system, or derivative, first ensure that these dependencies are there:

    $ sudo aptitude install cmake
    $ sudo aptitude install libgmp3-dev

Next we need to install podofo manually, which itself has a number of dependencies:

1 - Install [ZLib](http://zlib.net/):

    $ cd ~/src
    $ wget http://zlib.net/zlib-1.2.6.tar.gz
    $ tar zxvfs zlib-1.2.6.tar.gz
    $ cd zlib-1.2.6
    $ ./configure
    $ make
    $ sudo make install

2 - Install [Freetype2](http://www.freetype.org/freetype2/index.html):

    $ cd ~/src
    $ (Download from site)
    $ cd ./freetype-2.4.9
    $ make
    $ sudo make install

3 - Install [fontconfig](http://www.freedesktop.org/wiki/Software/fontconfig):

    $ cd ~/src
    $ wget http://www.freedesktop.org/software/fontconfig/release/fontconfig-2.9.0.tar.gz
    $ tar zxvfs fontconfig-2.9.0.tar.gz
    $ cd fontconfig-2.9.0
    $ ./configure
    $ make
    $ sudo make install

And finally podofo itself:

    $ cd ~/src
    $ (Get Podofo)
    $ cd ./podofo
    $ mkdir build
    $ cd build
    $ cmake ../
    $ make
    $ sudo make install

Unfortunately libboost is out of date on Ubuntu 10.04, so it needs to be compiled manually (version 1.49 at the time of this writing):

    $ cd boost_1_49
    $ sudo ./bootstrap.sh
    $ sudo ./b2 install

(Note: The above step will take quite a long time...)

Next, pull down the project:

    $ cd ~/src
    $ git clone git@github.com:catprintlabs/Imposer.git imposer
    $ cd ./imposer

Create a build directory within the project (this is ignored by .gitignore):

    $ mkdir build
    $ cd build

Run CMake to generate the makefiles:

    $ cmake ../

Then compile the program:

    $ make

The program is now under ./src/ directory with the name ospi. You can run it from here just by typing "./src/ospi"
If you want to install it, instruct make to do it:

    $ make install


## Mac OS X Installation

To make it easier to install the dpendencies, you can use a package manager like Homebrew.
First ensure the dependencies are there:

    $ brew install cmake
    $ brew install boost
    $ brew install podofo
    $ brew install gmp

If you run into build issues, you may need to run this first:

    $ brew update
    $ brew doctor

Next, pull down the project:

    $ cd ~/src
    $ git clone git@github.com:catprintlabs/Imposer.git imposer
    $ cd ./imposer

Create a build directory within the project (this is ignored by .gitignore):

    $ mkdir build
    $ cd build

Run CMake to generate the makefiles:

    $ cmake ../

Then compile the program:

    $ make

The program is now under ./src/ directory with the name ospi. You can run it from here just by typing "./src/ospi"
If you want to install it, instruct make to do it:

    $ make install

