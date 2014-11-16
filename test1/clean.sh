#!/bin/sh

. ./common

if [ $# != 1 ]; then
	echo Syntax: $0 test_directory_name ;
	exit 1;
fi

make clean
echo removing test directory $1
rm -r $1
rm -f  $OUTPUT_DIR/*
