PROJECT(brewtarget)
cmake_minimum_required(VERSION 2.6)

# Creates a Makefile.
# NOTE: cmake . -DCMAKE_INSTALL_PREFIX=/tmp/blah && make DESTDIR=/foo
# will install to /foo/tmp/blah.

# Version
set( brewtarget_VERSION_MAJOR 1 )
set( brewtarget_VERSION_MINOR 2 )
set( brewtarget_VERSION_PATCH 3 )

# Directories.
SET(SRCDIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Find Qt4 (>= 4.6.0) for this project.
FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtSvg QtWebkit QtXml Phonon REQUIRED)

# Tell cmake to process other CMakeList.txt's
ADD_SUBDIRECTORY(${SRCDIR})
