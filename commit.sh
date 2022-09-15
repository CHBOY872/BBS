#!/bin/sh

FILES="*.c *.h *.hpp *.cpp *.sh *.mk makefile PROTOCOL.txt
file/*.c file/*.h file/*.hpp file/*.cpp
user/*.c user/*.h user/*.hpp user/*.cpp
server/*.c server/*.h server/*.hpp server/*.cpp"

for FILE in $FILES
do 
    git add $FILE
done

if [ -z "$1" ]
then
    git commit
else 
    git commit -m "$1"
fi

if [ "$?" -eq "0" ]
then
    git push
fi