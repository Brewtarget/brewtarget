#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=

# Macros
PLATFORM=GNU-Linux-x86

# Include project Makefile
include brewtarget-Makefile.mk

# Object Directory
OBJECTDIR=build/Plugins/${PLATFORM}

# Object Files
OBJECTFILES=

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	cd ../../QtDesignerPlugins && qmake FermentableTableWidgetPlugin.pro && \
make && \
gksudo make install && \
make clean && \
qmake HopTableWidgetPlugin.pro && \
make && \
gksudo make install && \
make clean && \
qmake MiscTableWidgetPlugin.pro && \
make && \
gksudo make install && \
make clean && \
qmake MashStepTableWidgetPlugin.pro && \
make && \
gksudo make install && \
make clean

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	cd ../../QtDesignerPlugins && 

# Subprojects
.clean-subprojects:
