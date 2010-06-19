#!/bin/sh
#
SANDBOXCONFIG=sandbox-config.sh
if [ ! -f "$SANDBOXCONFIG" ]
then 
  echo "Content-type: text/html"
  echo
  echo "$SANDBOXCONFIG must be present"
  exit 0
fi
. ./$SANDBOXCONFIG
if [ -z "$BASE_DIR" -o -z "$BASE_URI" ]
then
  echo "Content-type: text/html"
  echo
  echo "$SANDBOXCONFIG must define BASE_DIR and BASE_URI"
  exit 0
fi

lua=lua
if [ -x $BASE_DIR/$lua ]
then
    LUA=$BASE_DIR/$lua
else
    if type $lua 2>/dev/null 1>&2
    then
        LUA=$lua
    else
        echo "Content-type: text/html"
        echo
        echo "no lua interpreter found"
        exit 0
    fi
fi

if [ xx = xxy ]
then
    echo "Content-type: text/html"
    echo
    echo "DEBUG-Output: lua interpreter $LUA found"
    exit 0
fi

$LUA $BASE_DIR/page_processor.lua 2>&1
