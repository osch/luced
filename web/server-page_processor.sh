#!/bin/sh


export BASE_DIR="/home/luced.de/htdocs"
export BASE_URI=""

CGI_BIN_DIR=/home/luced.de/cgi-bin

$CGI_BIN_DIR/lua $CGI_BIN_DIR/page_processor.lua
