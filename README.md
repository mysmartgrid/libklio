# LIBKLIO - A SIMPLE STORAGE LIBRARY FOR TIME-SERIES DATA

  https://github.com/mysmartgrid/libklio/

This library implements a storage layer for time-series data such as
meter readings. Its name references Klio, the greek muse of history[0].
The purpose of the library is to abstract from the storage backend. At
this time, a simple SQLite-based storage backend is implemented, but a
key-value based store will be implemented. The library is written in
C++ and intended to be used within other programs. All functionality is
tested within a seperate unit-test-driven testsuite.

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

Each commend comes with its own builtin help, so e.g.

    $ klio-store -h

provides this information:

    Usage: klio-store ACTION [additional options]:
    -h [ --help ]          produce help message
    -v [ --version ]       print libklio version and exit
    -a [ --action ] arg    Valid actions are: create
    -s [ --storefile ] arg the data store to use

A typical workflow would be:

1. Create a store.

    $ klio-store create teststore.db
    Attempting to create "teststore.db"
    Initialized store: SQLite3 database, stored in file "teststore.db"

2. Add a sensor definition.

    $ klio-sensor create -s teststore.db -i bullshitsensor -u heveling -z MEZ \
      -d "How much $foo is going on?" 
    opened store: SQLite3 database, stored in file "teststore.db"
    added: bullshitsensor(862acc00-9f18-4f58-9bcd-c4658f591d2d), unit heveling, 
    tz=MEZ, description: Unknown

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

    $ klio-export json -s teststore.db -i bullshit -o log.json
    opened store: SQLite3 database, stored in file "teststore.db"


## FEEDBACK
  If you encounter problems, please contact the developers on the
  mysmartgrid developer mailing list[1]. Alternatively, the issue tracker
  for this library lives on github[2].

## BUILDING
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

## BUGS 
The library was developed with great care, but we cannot guarantee that
no bugs are in the library. Please submit any issues using the issue
tracker[2].

## LICENSE
This library is (c) Fraunhofer ITWM and distributed under the terms of
the GPLv3.

## REFERENCES
[0] http://en.wikipedia.org/wiki/Clio
[1] http://listserv.mysmartgrid.de/mailman/listinfo/msg-dev
[2] https://github.com/mysmartgrid/libklio/issues
