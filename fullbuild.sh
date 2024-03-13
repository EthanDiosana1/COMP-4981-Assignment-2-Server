#!/bin/bash

./generate-cmakelists.sh

./change-compiler.sh -c clang

./build.sh
