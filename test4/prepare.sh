#!/bin/sh

. ./common

if [ $# != 1 ]; then
	echo Syntax: $0 test_directory_path ;
	exit 1;
fi

echo building utils 
make


basedir=$(pwd)

echo creating test directory $1
mkdir $1
cp teststr.tar.gz $1
cp -r bin $1

cd $1

tar -xzf teststr.tar.gz

echo generating outputs
cd $basedir
for g in $(ls $EXPECTED_DIR/*.gen 2> /dev/null)
do
	. ./$g
done 
