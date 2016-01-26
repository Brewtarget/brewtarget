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

If you want the conversion code to work, you will need a fairly recent perl
installed. You will also need the DBD drivers for SQLite and postgres
installed. Again, on ubuntu you need to install libdbd-pg-perl and
libdbd-sqlite3-perl as well as all of their dependencies. On gentoo, you need
to emerge dev-perl/DBD-Pg and dev-perl/DBD-SQLite.

Steps
=====

1. Install PostgreSQL. There are a bazillion guides for this. Find one, follow
it. 
2. Modify pg\_hba.conf to allow md5 authentication for both local and host
connections.
3. Modify pg\_hba.conf to bind to whatever IP addresses you want.
4. Create a user. I created one called brewtarget, mostly because I lack
imagination
5. Create a database. I've named mine brewtarget, in a fit of originality.
This document will assume you did the same, or that if you change it you are
smart enough to figure it out. Make the user you created in step 4 the owner.
This will automatically grant that user create/delete table access.
6. Build this branch.
7. Start brewtarget, open the options screen and set up the database
information.
8. Restart brewtarget. If I did it right, it should just work.

### Optional -- Import
I've written a perl script that can take the SQLite database and import it
into postgres. This is harder than it sounds, due to subtle differences
between the two products.
   $ try\_dbi.pl -h
   Usage: try\_dbi.pl -s sqlite db -p password \[-u username\] \[-h hostname\] \[-P port\] \[-n database\] \[-h\]
   where
      -s [sqlite db] -- the SQLite database file
      -p [password]  -- password for the user 
      -h [hostname]  -- hostname for the PgSQL server, defaults to localhost
      -P [port]      -- port number for the PgSQL server, defaults to 5432
      -n [database]  -- database name, defaults to "brewtarget"
      -u [username]  -- user with create table access in the PgSQL server,
                        defaults to brewtarget
      -h             -- prints this screen and exits
You nede to atleast provide the -s and -p flags. If you changed the username
or database name, you will need to provide those two.

## Run
Now you should be able to run brewtarget, using postgres as its data source.

Introduction
-------------
It's a brave new world of clouds and mobile devices. I have been slowly
burning cycles for the last year trying to get brewtarget ready. This is the
third step.

In moving to mobile and clouds, the hardest problem to solve has been the
database. SQLite is great for local access, and mighty fast. But it is
ultimately a file, and keeping that synchronized over multiple devices has
proven hard. 

Additionally, SQLite likes to have just one process accessing the DB at a time, which
has caused problems previously.

Whatever solution we used has to atleast address these two issues.

Dropbox, Google Drive and the rest
==================================
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
have written to, etc.

The second hardest problem was the credentials. Authenticating to the hsoting
service was non-trivial but, more importantly, I didn't want to get into
storing those credentials. People store very personal things in dropbox, and I
wasn't going to be responsible for them getting hacked.

The true hardest problem, though, was the mobile aspect. File systems are
tricky on Android devices, mostly because Google doesn't want you thinking in
those terms. It may have been my own mental instability, but I simply couldn't
wrap my head around where the database file actually had to go. 

Other concerns included the fact that we have seen databases get corrupted on
dropbox. What ever we do, we cannot lose user data.

MongoDB or other, cloud-based NoSQL databases
==========================================
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

PostgreSQL or MariaDB
======================
While researching other options, I found some services offering "free"
PostgreSQL servers hosted on AWS, Azure or Google. This struck me as the
perfect solution.

It would not lock you into any vendor. You can run your server on your own
equipment, on your own network. You can select any cloud service you like, and
install the server there. Or you can select a more full service offering like
mentioned above.

It would require only a minimal reworking of our code to connect to the
network service instead of the the SQLite database. The code isn't doing any
seriously hard SQL (no inner joins sort of crap), so once we got over some of
the particulars of the dialects it should just ... work.

And it mostly did.





