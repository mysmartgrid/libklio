#!/bin/bash

#
# Add field device_type_id to the sensors table.
#

if [ $# != 2 ]; then
  echo
  echo "Usage: patch.13032014.sh  <database file path to convert>  <converted database file path>"
  echo
  exit 1
fi

ORIGINAL_FILE=$1
CONVERTED_FILE=$2

if [ ! -f $ORIGINAL_FILE ]; then
    echo "The original database file could not be found."
    exit 1
fi

if [ -f $CONVERTED_FILE ]; then
    echo "The path informed for the converted database must not exist yet."
    exit 1
fi

cp $ORIGINAL_FILE $CONVERTED_FILE

if [ $? -eq 0 ]; then

    echo "ALTER TABLE sensors ADD COLUMN device_type_id INTEGER;" | sqlite3 $CONVERTED_FILE

    if [ $? -eq 0 ]; then

        echo "UPDATE sensors SET device_type_id = 0;" | sqlite3 $CONVERTED_FILE

        if [ $? -eq 0 ]; then
            echo "The patch has been successfully applied."
            exit 0
        fi
    fi
fi

if [ -f $CONVERTED_FILE ]; then
    rm $CONVERTED_FILE
fi

echo "The patch could not be applied."
exit 1




