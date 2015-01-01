#!/bin/bash

# sqlitediff.sh file.sqlite commit-a commit-b
DBFILE=$1
COMMITA=$2
COMMITB=$3

SQLITEA=$(mktemp)
SQLITEB=$(mktemp)
DUMPA=$(mktemp)
DUMPB=$(mktemp)

git show "$COMMITA:$DBFILE" > "$SQLITEA"
git show "$COMMITB:$DBFILE" > "$SQLITEB"

sqlite3 "$SQLITEA" .dump > $DUMPA
sqlite3 "$SQLITEB" .dump > $DUMPB

git diff --no-index -- "$DUMPA" "$DUMPB"

rm $SQLITEA
rm $SQLITEB
rm $DUMPA
rm $DUMPB
