## LIBKLIO - A SIMPLE STORAGE LIBRARY FOR TIME-SERIES DATA

  https://github.com/mysmartgrid/libklio/

This library implements a storage layer for time-series data such as
meter readings. Its name references Klio, the greek muse of history[0].
The purpose of the library is to abstract from the storage backend. At
this time, three types of storage backend are supported: SQLite, RocksDB, and a
remote storage on the mySmartGrid website. The library is written in C++ and
intended to be used within other programs. All functionality is tested within
a seperate unit-test-driven testsuite.

## CONCEPTS
The library uses several concepts to make the storage of time-series
data as easy as possible. A *store* is a storage backend which can store
data for several *sensors*. A sensor is a source of time-series data.
Its name must be unique and is also associated with a timezone.
Internally, only unix timestamps are stored. Each sensor can manage
tuples of time and values. The store is responsible for storing the data
along with the metadata in a consistent manner.

## USAGE
Please see the example programs and test programs for usage examples.
Some utilities are included in the distribution:

1. klio-store: A tool to manipulate stores.
2. klio-sensor: Tool for the manipulation of sensors and sensor data.
3. klio-export: Export the stored data in several different formats.

Each command comes with its own builtin help, so e.g.

    $ klio-store -h

provides this information:

    Usage: klio-store ACTION [additional options]:
      -h [ --help ]            produce help message
      -v [ --version ]         print libklio version and exit
      -a [ --action ] arg      Valid actions are: create, check, sync, upgrade
      -s [ --storefile ] arg   the data store to use
      -r [ --sourcestore ] arg the data store to use as source for synchronization


A typical workflow would be:

1. Create a store.

        $ klio-store create teststore.db
        Attempting to create "teststore.db"
        Initialized store: SQLite3 database, stored in file "teststore.db"

2. Add a sensor definition.

        $ klio-sensor create -s teststore.db -i bullshitsensor -u heveling \
          -z "Europe/Berlin" -d "How much $foo is going on?"
        opened store: SQLite3 database, stored in file "teststore.db"
        added: bullshitsensor(862acc00-9f18-4f58-9bcd-c4658f591d2d), unit heveling, 
        tz=Europe/Berlin, description: How much $foo is going on?

   You must provide a valid timezone for each sensor - the command will
   list valid timezones if you don't specify one. It is not possible to
   specify MESZ etc, because the change between daylight saving time an
   normal time is dependent on the geographic location (can be one day
       in Germany, and two weeks later in France).

3. Add a value 42 with the unix timestamp 23 manually (typically, another 
    program would deal with this):

        $ klio-sensor addreading -s teststore.db -i bullshitsensor -t 23 -r 42
        opened store: SQLite3 database, stored in file "teststore.db"
        Added reading to sensor bullshitsensor

4. Query the sensor:

        $ klio-sensor info -s teststore.db -i bullshitsensor
        opened store: SQLite3 database, stored in file "teststore.db"
        Info for sensor bullshitsensor
         * description:Unknown
         * uuid:862acc00-9f18-4f58-9bcd-c4658f591d2d
         * unit:heveling
         * timezone:MEZ
         * 1 readings stored

5. Export sensor data:

        $ klio-export json -s teststore.db -i bullshitsensor -o log.json
        opened store: SQLite3 database, stored in file "teststore.db"


## FEEDBACK
  If you encounter problems, please contact the developers on the
  mysmartgrid developer mailing list[1]. Alternatively, the issue tracker
  for this library lives on github[2].

## INSTALLATION
So far, the library has been successfully compiled on Ubuntu Linux
11.10, Debian Linux 6.0 and Mac OS 10.7 (Lion). The library depends on 
several libraries:

1. Boost version 1.48.0 or later
2. sqlite version 3.7.15 or later
3. pkg-config 0.28 or later

Optionally, in case you want to use the mySmartGrid web site as the storage
backend, the following additional package needs to be installed. This is your 
case if you are compiling Hexabus.

1. libmysmartgrid (https://github.com/mysmartgrid/libmysmartgrid)

Optionally, in case you want to use RocksDB as the storage backend, the
following additional package needs to be installed.

1. RocksDB 2.0 or later

Optionally, in case you want to use Redis as the storage backend, the
following additional package needs to be installed.

1. Redis3m (https://github.com/luca3m/redis3m)

requirements for redis3m:
   * libhiredis-dev
   * libmsgpack-dev
   * boost

In order to run unit tests, a Redis server must be accessible from the
client machine via host name redis-server1 and port 6379.

Optionally, in case you want to use PostgreSQL as the storage backend,
the following additional packages need to be installed on the client machine.

1. postgresql client 9.3 or later
2. libpq-dev 9.3 or later

In order to run unit tests, a PostgreSQL sertver must be accessible from the
client machine via host name postgresql-server1 and port 5432. In addition, a
database named kliostore must be created on that server, belonging to the user
kliouser, with password 12test34.


The library uses CMake for compile-time configuration. A Makefile is
provided which wraps the build infrastructure. A simple

    $ make

should suffice to compile the library if all dependencies are available.
Afterwards, you can use

    $ make install

to install the library. If you prefer proper packaging, you can also
build installation packages in several formats:

    $ make release

builds the package with release settings (i.e. full optimization, no
debug symbols) and places the created packages in the 'build'
subdirectory.

Alternativ way to build libklio:
    $ mkdir build
    $ (cd build && cmake <libklio_source_dir> && make)
    

## BUGS 
The library was developed with great care, but we cannot guarantee that
no bugs are in the library. Please submit any issues using the issue
tracker[2].

## LICENSE
This library is (c) Fraunhofer ITWM and distributed under the terms of
the GPLv3.

## REFERENCES
 * [0] http://en.wikipedia.org/wiki/Clio
 * [1] http://listserv.mysmartgrid.de/mailman/listinfo/msg-dev
 * [2] https://github.com/mysmartgrid/libklio/issues
