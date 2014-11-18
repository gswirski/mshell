#!/bin/sh

. ./common

if [ $# != 3 ]; then 
	echo Syntax: $0 shell_path test_directory test_number;
	exit 1;
fi

base_dir=$PWD
shell=$1
test_directory=$2
tn=$3

cd $test_directory

echo -n "Test " $tn ": " 
head -1 $base_dir/$INPUT_DIR/$tn.in
echo -n "Command:"

inf=$base_dir/$INPUT_DIR/$tn.in
outf=$base_dir/$OUTPUT_DIR/$tn.out
errf=$base_dir/$OUTPUT_DIR/$tn.err

if [ -f $base_dir/$EXEC_DIR/$tn.exec ]; then
	echo "specific see " $base_dir/$EXEC_DIR/$tn.exec
	. $base_dir/$EXEC_DIR/$tn.exec
else
	$shell < $inf > $outf 2> $errf
	echo $shell "<" $base_dir/$INPUT_DIR/$tn.in ">" $base_dir/$OUTPUT_DIR/$tn.out "2>"$base_dir/$OUTPUT_DIR/$tn.err
fi

diff $base_dir/$OUTPUT_DIR/$tn.out $base_dir/$EXPECTED_DIR/$tn.out > /dev/null
result1=$?

result2=0
if [ -f $base_dir/$EXPECTED_DIR/$tn.err ]; then
	diff $base_dir/$OUTPUT_DIR/$tn.err $base_dir/$EXPECTED_DIR/$tn.err > /dev/null
	result2=$?
fi

if [ $result1 -eq 0 ] && [ $result2 -eq 0 ]; 
	then echo OK; 
	else echo FAIL ;
fi	
