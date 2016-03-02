HowTo
-----
You can read the [Introduction][Introduction] for more information on why and
how. But most people want the tl;dr so here it is.

Libraries and Requirements
==========================
For brewtarget to run, you will need to make sure you have the Qt PSQL
libraries installed. On ubuntu, install the libqt5sql5-psql package. On
gentoo, emerge dev-qt/qtsql with the postgres use flag. I don't work with
other OSes, but the rough idea should be the same.

Due to needing/wanting certain abilities in PostgreSQL, we support v9.5. You
may have to jump through some hoops to make that available on your OS of
choice. As with all things sysadmin, google is your friend. It is a very fresh
release. If this causes problems, I can attempt to backlevel.

Steps
=====

1. Install PostgreSQL. There are a bazillion guides for this. Find one, follow
it. 
2. Modify pg\_hba.conf to allow md5 authentication for both local and host
   connections.
3. Modify pg\_hba.conf to bind to whatever IP addresses you want.
4. Connect to postgresql: psql -U postgres
5. Create a user: create user [username] with password 'password'; 
   I created one called brewtarget, mostly because I lack imagination
6. Create a database: create database brewtarget with owner brewtarget; 
   I've named mine brewtarget, in a fit of originality.  This document will
   assume you did the same, or that if you change it you are smart enough to
   figure it out.  If you created a user in step 5, make sure they are the
   owner of the database.  This will automatically grant that user
   create/delete table access.
7. Build this branch.
8. Start brewtarget, open the options screen and set up the database
   information. Keep the schema to public for now. That may go away at some
   point.
9. When asked, say you want to automatically copy the data. This should work,
   but it may take some time. If it doesn't, it will spit an error message on
   the console that I NEED in order to know what broke.
10. Restart brewtarget. It will be slower to start than using SQLite

## What works
  o Recipe CRUD (create/read/update/delete)
  o brewnotes, aka, Brew It!
  o Creating/deleting/updating elements
  o Copying existing ingredients
  o PostgreSQL remote and localhost -- I haven't tried cloud systems, but they should work
  o Automatically copying information from SQLite -> PostgreSQL
  o Automatically copying information from PostgreSQL -> PostgreSQL
  o Automatically copying information from PostgreSQL -> SQLite
  o Configuration screens for setting up remote dbs

## What may not work (not tested)
  o Inventory
  o Reordering/adding instruction steps
  o Integrating new ingredients from an updated SQLite database

## What won't work
  o Backup copies just don't make sense anymore
  o Saving -- all updates are written automatically.

## Known Issues
  o sqlite is much faster. I can tell simply from the delay at startup how I'm
    configured. Of course, I have spent exactly 0 seconds trying to optimize
    postgresql.
  o I wonder if we shouldn't attempt to restart brewtarget automatically after
    step 9?

## Some tricks
  o If you want to quickly reset, just remove the db\* variables from your
    config file. brewtarget will default back to your sqlite file. You can then
    drop the psql database and recreate it. I've done this many, many
    times.

#Introduction

It's a brave new world of clouds and mobile devices. I have been slowly
burning cycles for the last year trying to get brewtarget ready. This is the
third step. No worries, the pelvic thrust will still drive you insane.

In moving to mobile and clouds, the hardest problem to solve has been the
database. SQLite is great for local access, and mighty fast. But it is
ultimately a file, and keeping that synchronized over multiple devices has
proven hard. 

Additionally, SQLite likes to have just one process accessing the DB at a time, which
has caused problems previously.

Whatever solution we used has to at least address these two issues.

##Dropbox, Google Drive and the rest

One option was to keep the SQLite database file, and use an external service
to synchronize the file. 

This had the lowest possible impact. We already have code in place to make
backup copies and to move the SQLite db file. It should have been a simple
matter to introduce the code to copy the dbfile from the hosting service and
just continue as normal. Initial investigations found this approach to be
much harder than expected.

The hosting services are mostly written for easy access via Java or
Javascript, not C++. There are a few third part helper classes for Qt, but they are
poorly documented. Most of the hosting services use REST+JSON, which we could
have written to, etc. But it would have required a number of new classes, and
more error handling than I think I want to think of.

The second hardest problem was the credentials. Authenticating to the hosting
service was non-trivial but, more importantly, I didn't want to get into
storing those credentials. People store very personal things in dropbox, and I
wasn't going to be responsible for them getting hacked.

The true hardest problem, though, was the mobile aspect. File systems are
tricky on Android devices, mostly because Google doesn't want you thinking in
those terms. It may have been my own mental instability, but I simply couldn't
wrap my head around where the database file actually had to go and how to get
the dropbox APIs to put something somewhere that brewtarget could find it.

Other concerns included the fact that we have seen databases get corrupted on
dropbox. What ever we do, we cannot lose user data.

##MongoDB or other, cloud-based NoSQL databases

Another possible option was to transition to a complete cloud based solution
with a nosql database. Our datasets are not horribly large, and this would
solve many of the issues with synchronizing the SQLite file.

By isolating the data, exposing the passwords would not be quite as horrible.
Since there would be no files, there was nothing to synchronize or to worry
about where on a mobile device it went.

My biggest issue with this approach was I didn't want to lock people into
using just one provider. The NOSQL databases tend to have very different
approaches, layouts and interfaces.  They also all used REST+JSON as their
primary interfaces, so we would still need to make a series of classes.

Another major issue was that we would have to seriously rework all of the
database interfaces. The code base has done a fairly good job of isolating all
of these, but it would still be a significant undertaking.

##PostgreSQL or MariaDB

While researching other options, I found some services offering "free"
PostgreSQL servers hosted on AWS, Azure or Google. This struck me as the
perfect solution.

It would not lock the user into any vendor. You can run your server on your own
equipment, on your own network. You can select any cloud service you like, and
install the server there. Or you can select a more full service offering like
mentioned above.

It should require only a minimal reworking of our code to connect to the
network service instead of the the SQLite database. The code isn't doing any
seriously hard SQL (no inner joins sort of crap), so once we got over some of
the particulars of the dialects it should just ... work.

And it mostly did. The biggest problem I ran into was that SQLite is a little
... well, okay, a lot loose. It doesn't seem to actually enforce size limits,
it doesn't mind if numeric values are enclosed in quotes, booleans are
represented as integers, etc. Oddly, it was the last element that was the
hardest to fix. delete and display are *everywhere*.

### Why PostgreSQL?
No real reason, really. I'm just slightly more comfortable with postgreSQL
than I am with mariadb. I had to start somewhere, and so I did. I think I will
still try my hand at mariadb. Having done it once, it should be easy to do it
twice, right?

So I tried that. It didn't work so well. It seems "use" is a keyword in
mariadb, which causes problems for the hop table and the misc table. The
really fun part is that is one of the BeerXML defined attributes that we are
no supposed to change. So I guess mariadb is on the back burner until somebody
has a brilliant idea.

### Why PostgreSQL 9.5
This coding effort was started on Jan 22, 2016. PostgreSQL 9.5 was released on
Jan 6, 2016. I would not normally be so close to the bleeding edge. The
inventory tables, though, used SQLite's "insert or update" functionality.
PostgreSQL didn't have anything similar until 9.5.


