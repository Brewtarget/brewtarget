#!/bin/bash

BINDIR=/usr/local/bin/
FILEDIR=~/.brewtarget

cp brewtarget "$BINDIR" && \
mkdir "$FILEDIR" && \
cp database.xml recipes.xml mashs.xml "$FILEDIR"

