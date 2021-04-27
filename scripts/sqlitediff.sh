#!/bin/bash

# sqlitediff.sh file.sqlite commit-a commit-b
# To compare commit to current (uncommitted) db:
# sqlitediff.sh file.sqlite commit-a
# To compare HEAD to current uncommitted db:
# sqlitediff.sh file.sqlite

DBFILE=$1
COMMITA=$2
COMMITB=$3

SQLITEA=$(mktemp)
SQLITEB=$(mktemp)
DUMPA=$(mktemp)
DUMPB=$(mktemp)

git show "$COMMITA:$DBFILE" > "$SQLITEA"
if [ $# -ge 3 ]
then
	git show "$COMMITB:$DBFILE" > "$SQLITEB"
else
	cp "$DBFILE" "$SQLITEB"
fi

sqlite3 "$SQLITEA" .dump > $DUMPA
sqlite3 "$SQLITEB" .dump > $DUMPB

git diff --no-index -- "$DUMPA" "$DUMPB"

rm $SQLITEA
rm $SQLITEB
rm $DUMPA
rm $DUMPB
