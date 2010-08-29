PROJECT(brewtarget)
cmake_minimum_required(VERSION 2.8)

# Creates a Makefile.
# NOTE: cmake . -DCMAKE_INSTALL_PREFIX=/tmp/blah && make DESTDIR=/foo
# will install to /foo/tmp/blah.

# Version
SET( brewtarget_VERSION_MAJOR 1 )
SET( brewtarget_VERSION_MINOR 2 )
SET( brewtarget_VERSION_PATCH 3 )

SET( brewtarget_VERSION_STRING "${brewtarget_VERSION_MAJOR}.${brewtarget_VERSION_MINOR}.${brewtarget_VERSION_PATCH}" )

# Directories.
SET(SRCDIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Find Qt4 (>= 4.6.0) for this project.
FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtSvg QtWebkit QtXml Phonon REQUIRED)

# Tell cmake to process other CMakeList.txt's
ADD_SUBDIRECTORY(${SRCDIR})
