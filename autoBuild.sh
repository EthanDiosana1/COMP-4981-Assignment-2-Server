#!/bin/bash

DIRECTORY1=./src
DIRECTORY2=./include
EXCLUDE_DIRECTORY=./exclude

SCRIPT=./fullbuild.sh

while true; do
    inotifywait -e close_write -r "$DIRECTORY1" "$DIRECTORY2" --exclude "$EXCLUDE_DIRECTORY"
    "$SCRIPT"
done
