#!/bin/sh

FILES="*.c *.h *.hpp *.cpp *.sh makefile PROTOCOL.txt"

for FILE in $FILES
do 
    git add $FILE
done

if [[ -z $1 ]]
then
    git commit
else 
    git commit -m $1
fi