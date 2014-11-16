#!/bin/sh

if [ $# != 1 ]; then
	echo Syntax: $0 test_directory_path ;
	exit 1;
fi

make

echo creating test directory $1
mkdir $1
cp teststr.tar.gz $1

cd $1

tar -xzf teststr.tar.gz
