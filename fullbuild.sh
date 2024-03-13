#!/bin/bash

rm -rf ./build

./generate-cmakelists.sh

./change-compiler.sh -c clang

./build.sh
