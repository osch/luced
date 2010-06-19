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
$BASE_DIR/lua $BASE_DIR/page_processor.lua 2>&1
