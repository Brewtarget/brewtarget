# Brewtarget

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
* Dan Cavanagh <dan@dancavanagh.com>
* Rob Taylor <robtaylor@floopily.org>
* Kregg K <gigatropolis@yahoo.com>
* A.J. Drobnich <aj.drobnich@gmail.com>
* Ted Wright <tedwright@users.sourceforge.net>
* Charles Fourneau (plut0nium) <charles.fourneau@gmail.com>
* Maxime Lavigne (malavv) <duguigne@gmail.com>
* Peter Buelow <goballstate@gmail.com>
* David Grundberg <individ@acc.umu.se>
* Tim Payne <swstim@gmail.com>
* Samuel Östling <MrOstling@gmail.com>
* Luke Vincent <luke.r.vincent@gmail.com>
* Eric Tamme <etamme@gmail.com>
* Julein <j2bweb@gmail.com>
* Jeff Bailey <skydvr38@verizon.net>
* Piotr Przybyla (przybysh) <przybysh@gmail.com>
* Chris Hamilton <marker5a@gmail.com>

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

Most of these can be satisfied by installing qt and a compiler.

* libc6 (>= 2.14)
* libgcc1 (>= 1:4.1.1)
* libphonon4 (>= 4:4.2.0)
* libqt4-network (>= 4:4.5.3)
* libqt4-sql (>= 4:4.5.3)
* libqt4-svg (>= 4:4.5.3)
* libqt4-xml (>= 4:4.5.3)
* libqt4-xmlpatterns (>= 4:4.5.3)
* libqtcore4 (>= 4:4.8.0)
* libqtgui4 (>= 4:4.8.0)
* libqtwebkit4 (>= 2.2~2011week36)
* libstdc++6 (>= 4.4.0)
* phonon
* libqt4-sql-sqlite

### Build Dependencies

* cmake (>= 2.8)
* autotools-dev
* libqt4-dev
* qt4-qmake
* libphonon-dev
* libqtwebkit-dev
* sqlite3

### Compiling

    $ mkdir /tmp/brewtarget-build
    $ cd /tmp/brewtarget-build
    $ cmake /path/to/brewtarget
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
  Makes .deb, .rpm, and .tar.bz2 binary packages.
* `make package_source`
  Makes a .tar.bz2 source package.
* `make source_doc`
  Makes html documentation of the source in doc/html.
* `make translations`
  If the cmake option `UPDATE_TRANSLATIONS` is set, updates the `*.ts` files
  and creates .qm files. Otherwise, just creates .qm files from `*.ts` files.

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
* `ENABLE_PROFILING` - `OFF` by default. If `ON`, builds with 
  profiling compiler flags.
* `NO_PHONON` - `OFF` by default. If `ON`, does not build any Phonon code.
* `NO_MESSING_WITH_FLAGS` - `OFF` by default. `ON` means do not add any build
   flags whatsoever. May override other options.

