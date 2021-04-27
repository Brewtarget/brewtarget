# Brewtarget

![Linux Workflow](https://github.com/brewtarget/brewtarget/actions/workflows/linux-ubuntu.yml/badge.svg)

![Windows Workflow](https://github.com/brewtarget/brewtarget/actions/workflows/windows.yml/badge.svg)

Brewtarget is free open-source brewing software, and a beer recipe creation
tool available for Linux, Mac, and Windows. It automatically calculates color,
bitterness, and other parameters for you while you drag and drop ingredients
into the recipe. Brewtarget also has many other tools such as priming sugar
calculators, OG correction help, and a unique mash designing tool. It also can
export and import recipes in BeerXML, allowing you to easily share recipes with
friends who use BeerSmith or other programs. All of this means that Brewtarget
is your single, free, go-to tool when crafting your beer recipes.

## Authors

* Adam Hawes <ach@hawes.net.au>
* Aidan Roberts <aidanr67@gmail.com>
* A.J. Drobnich <aj.drobnich@gmail.com>
* André Rodrigues <andre@sabayon.local>
* Blair Bonnett <blair.bonnett@gmail.com>
* Brian Rower <brian.rower@gmail.com>
* Carles Muñoz Gorriz <carlesmu@internautas.org>
* Chris Pavetto <chrispavetto@gmail.com>
* Chris Speck <cgspeck@gmail.com>
* Dan Cavanagh <dan@dancavanagh.com>
* Daniel Moreno <danielm5@users.noreply.github.com>
* Daniel Pettersson <pettson81@gmail.com>
* David Grundberg <individ@acc.umu.se>
* Eric Tamme <etamme@gmail.com>
* Greg Greenaae <ggreenaae@gmail.com>
* Greg Meess <Daedalus12@gmail.com>
* Idar Lund <idarlund@gmail.com>
* Jamie Daws <jdelectronics1@gmail.com>
* Jean-Baptiste Wons <wonsjb@gmail.com>
* Jeff Bailey <skydvr38@verizon.net>
* Jerry Jacobs <jerry@xor-gate.org>
* Jonatan Pålsson <jonatan.p@gmail.com>
* Jonathon Harding <github@jrhardin.net>
* Julian Volodia <julianvolodia@gmail.com>
* Kregg Kemper <gigatropolis@yahoo.com>
* Luke Vincent <luke.r.vincent@gmail.com>
* Marcel Koek <koek.marcel@gmail.com>
* Mark de Wever <koraq@xs4all.nl>
* Markus Mårtensson <mackan.90@gmail.com>
* Matt Anderson <matt.anderson@is4s.com>
* Mattias Måhl <mattias@kejsarsten.com>
* Matt Young <mfsy@yahoo.com>
* Maxime Lavigne <duguigne@gmail.com>
* Medic Momcilo <medicmomcilo@gmail.com>
* Mike Evans <mikee@saxicola.co.uk>
* Mik Firestone <mikfire@gmail.com>
* Mikhail Gorbunov <mikhail@sirena2000.ru>
* Mitch Lillie <mitch@mitchlillie.com>
* Padraic Stack <padraic.stack@gmail.com>
* Peter Buelow <goballstate@gmail.com>
* Peter Urbanec <git.user@urbanec.net>
* Philip Greggory Lee <rocketman768@gmail.com> -- Original developer
* Rob Taylor <robtaylor@floopily.org>
* Samuel Östling <MrOstling@gmail.com>
* Scott Peshak <scott@peshak.net>
* Théophane MARTIN <theophane.m@gmail.com>
* Tyler Cipriani <tcipriani@wikimedia.org>

Author list created with the help of the following command:

    $ git log --raw | grep "^Author: " | sort -u

## Websites

### For Users

* [Main website](http://www.brewtarget.org) (No longer updated and has some out-of-date links)
* [Help group](https://groups.google.com/forum/?fromgroups=#!forum/brewtarget-help) (Linked to from the website, but it's better to raise issues on GitHub than post here)
* [Latest builds](https://github.com/Brewtarget/brewtarget/actions)
* [Brewtarget PPA](https://launchpad.net/~brewtarget-devs/+archive/ubuntu/brewtarget-releases) (out of date)
* [Bug tracker](https://github.com/Brewtarget/brewtarget/issues)

Latest builds are available by logging into Github, following the "Latest builds" link above, drilling down into the relevant OS and downloading the installer package.

### For Developers

* [Source code repository](https://github.com/Brewtarget/brewtarget)
* [Developers team](https://launchpad.net/~brewtarget-devs) (No longer used)
* [Developers wiki](https://github.com/Brewtarget/brewtarget/wiki)

## Compiling and Installing

### Dependencies

On Debian systems like Ubuntu, the packages for dependencies are:

* cmake (>= 2.8.11)
* git
* libxerces-c-dev
* libxerces-c-doc
* libxalan-c-dev
* libxalan-c-doc
* qtbase5-dev
* qttools5-dev
* qttools5-dev-tools
* qtmultimedia5-dev
* libqt5sql5-sqlite
* libqt5sql5-psql
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
