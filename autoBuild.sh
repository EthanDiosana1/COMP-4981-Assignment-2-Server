#!/bin/bash

FILE=./src/main.c

SCRIPT=./fullbuild.sh

while true; do
    inotifywait -e close_write "$FILE"
    "$SCRIPT"
done

