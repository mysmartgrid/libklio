#!/bin/bash

if [ $# != 1 ]; then
  echo "Inform the SQLite3 database file to be converted to the new version of libklio:"
  echo "     ./patch.13032014.sh <database file path>"
  exit 1
fi

FILE=$1

echo "ALTER TABLE sensors ADD COLUMN device_type_id INTEGER; UPDATE sensors SET device_type_id = 0;" | sqlite3 $FILE

if [ $? -eq 0 ]; then
  echo "The patch has been successfully applied."
else
  echo "The patch could not be applied."
fi
