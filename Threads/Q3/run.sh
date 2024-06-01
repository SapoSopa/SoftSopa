#!/bin/sh

#set -xe

clear
if [ -f "$1" ]; then
    rm "$1"
fi
gcc "$1".c -Wall -Wpedantic -Wextra -o "$1"
./"$1"
