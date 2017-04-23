# Brewtarget ![Build Status](https://travis-ci.org/Brewtarget/brewtarget.svg?branch=master)

Brewtarget is free open-source brewing software, and a beer recipe creation
tool available for Linux, Mac, and Windows. It automatically calculates color,
bitterness, and other parameters for you while you drag and drop ingredients
into the recipe. Brewtarget also has many other tools such as priming sugar
calculators, OG correction help, and a unique mash designing tool. It also can
export and import recipes in BeerXML, allowing you to easily share recipes with
friends who use BeerSmith or other programs. All of this means that Brewtarget
is your single, free, go-to tool when crafting your beer recipes.

## Authors

* Philip G. Lee <rocketman768@gmail.com> - Lead developer
* Mik Firestone <mikfire@gmail.com>
* Maxime Lavigne <duguigne@gmail.com>
* Theophane Martin <theophane.m@gmail.com>
* Dan Cavanagh <dan@dancavanagh.com>
* Rob Taylor <robtaylor@floopily.org>
* Kregg K <gigatropolis@yahoo.com>
* A.J. Drobnich <aj.drobnich@gmail.com>
* Ted Wright <tedwright@users.sourceforge.net>
* Charles Fourneau (plut0nium) <charles.fourneau@gmail.com>
* Samuel Östling <MrOstling@gmail.com>
* Peter Buelow <goballstate@gmail.com>
* David Grundberg <individ@acc.umu.se>
* Daniel Pettersson <pettson81@gmail.com>
* Tim Payne <swstim@gmail.com>
* Luke Vincent <luke.r.vincent@gmail.com>
* Eric Tamme <etamme@gmail.com>
* Chris Pavetto <chrispavetto@gmail.com>
* Markus Mårtensson <mackan.90@gmail.com>
* Julein <j2bweb@gmail.com>
* Jeff Bailey <skydvr38@verizon.net>
* Piotr Przybyla (przybysh) <przybysh@gmail.com>
* Chris Hamilton <marker5a@gmail.com>
* Julian Volodia <julianvolodia@gmail.com>
* Jerry Jacobs <jerry@xor-gate.org>
* Greg Meess <Daedalus12@gmail.com>

Author list created with:

    $ git log --raw | grep "^Author: " | sort | uniq -c | sort -nr

## Websites

### For Users

* [Main website](http://www.brewtarget.org)
* [Help group](https://groups.google.com/forum/?fromgroups=#!forum/brewtarget-help)
* [Brewtarget PPA](https://launchpad.net/~brewtarget-devs/+archive/ubuntu/brewtarget-releases)
* [Bug tracker](https://github.com/Brewtarget/brewtarget/issues)

### For Developers

* [Source code repository](https://github.com/Brewtarget/brewtarget)
* [Daily builds](https://launchpad.net/~brewtarget-devs/+archive/ubuntu/brewtarget)
* [Developers team](https://launchpad.net/~brewtarget-devs)
* [Developers wiki](https://github.com/Brewtarget/brewtarget/wiki)

## Compiling and Installing

### Dependencies

On Debian systems like Ubuntu, the packages for dependencies are:

* cmake (>= 2.8.11)
* git
* qtbase5-dev
* qttools5-dev
* qttools5-dev-tools
* qtmultimedia5-dev
* libqt5webkit5-dev
* libqt5sql5-sqlite
* libqt5sql-psql
* libqt5svg5-dev
* libqt5multimedia5-plugins
* doxygen (optional, for source documentation)

### Compiling

We do not do any in-source builds. You will create a separate directory
for the build.

    $ mkdir brewtarget-build
    $ cd brewtarget-build
    $ cmake /path/to/brewtarget-src
    $ make

### Installing

Linux-like systems may simply do:

    $ sudo make install

Systems that use .deb or .rpm packages may also create a package first:

    $ make package

Then either

    $ sudo dpkg -i brewtarget*.deb

or

    $ sudo rpm -i brewtarget*.rpm

On Mac and Windows environments, the `package` target will create an installer
that may be executed to finish the installation.

### Make targets

* `make package`
  Makes .deb, .rpm, NSIS Installer, and .tar.bz2 binary packages.
* `make package_source`
  Makes a .tar.bz2 source package.
* `make source_doc`
  Makes html documentation of the source in doc/html.

### Cmake options

These options are passed to `cmake` with the `-D` flag before compiling. For
example:

    $ cmake /path/to/brewtarget -DCMAKE_INSTALL_PREFIX=/usr -DDO_RELEASE_BUILD=ON

* `CMAKE_INSTALL_PREFIX` - `/usr/local` by default. Set this to `/usr` on
  Debian-based systems like Ubuntu.
* `BUILD_DESIGNER_PLUGINS` - `OFF` by default. If set to `ON`, builds the Qt Designer
  plugins instead of brewtarget.
* `DO_RELEASE_BUILD` - `OFF` by default. If `ON`, will do a release build.
  Otherwise, debug build.
* `NO_MESSING_WITH_FLAGS` - `OFF` by default. `ON` means do not add any build
   flags whatsoever. May override other options.
