#!/bin/bash


if [ "$#" -ne 1 ];
then
	echo "Usage: $0 <code tar>"
	exit
fi

TAR_FILE=$1

tar xvf ${TAR_FILE}
if [ $? -ne 0 ];
then
        echo "untar error"
        exit
fi


if [ ! -d "protocols" ];
then
	echo "protocols/ doesn't exist"
	exit
fi

if [ ! -d "project4" ];
then
	echo "project4/ doesn't exist"
	exit
fi

cp -rf protocols project4/

cd project4/
make clean
make

if [ $? -ne 0 ];
then
	echo "Compile error"
	exit
fi

declare -a TRACES=("traces/experiment1" "traces/experiment2" "traces/experiment3" "traces/experiment4" "traces/experiment5" "traces/experiment6" "traces/experiment7" "traces/experiment8")
declare -a PROTOCOLS=("MSI" "MESI" "MOSI" "MOESI" "MOESIF")

rm -rf verify.out
for p in "${PROTOCOLS[@]}"
do
	echo "-----------------------------"
	echo "Verifying protocol: $p"
	for t in "${TRACES[@]}"
	do
		./sim_trace -p $p -t $t > output.log 2>&1
		echo " " >> verify.out
		echo $p "  " $t >> verify.out
		tail -5 output.log >> verify.out
	done
done

echo "-----------------------------"


