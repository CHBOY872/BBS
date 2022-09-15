#!/bin/sh

DIRECTORIES=$(ls -F | grep /)

make all
for DIRECTORY in $DIRECTORIES
do
    rm -f $DIRECTORY*.o
done
