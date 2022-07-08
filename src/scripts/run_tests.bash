#!/usr/bin/env bash

sort=$1
if [ ! -z "$sort" ];
then
	delay=5
	if [ ! -z "$2" ]; 
	then
		delay=$2
	fi
	number=512
	if [ ! -z "$3" ]; 
	then
		number=$3
	fi
	"./$sort" "$delay" "$number" random;
	sleep 1;
	"./$sort" "$delay" "$number" reverse;
	sleep 1;
	"./$sort" "$delay" "$number" noshuffle;
	sleep 1;
	"./$sort" "$delay" "$number" slightshuffle;
	sleep 1;
	"./$sort" "$delay" "$number" shuffletail;
	sleep 1;
	"./$sort" "$delay" "$number" shufflehead;
	sleep 1;
	"./$sort" "$delay" "$number" mergepass;
	sleep 1;
	"./$sort" "$delay" "$number" quarters;
	sleep 1;
	"./$sort" "$delay" "$number" reversemergepass;
	sleep 1;
	"./$sort" "$delay" "$number" mountain;
	sleep 1;
	"./$sort" "$delay" "$number" radixpass;
	sleep 1;
	"./$sort" "$delay" "$number" binarysearchtree;
	sleep 1;
	"./$sort" "$delay" "$number" heapified;
	sleep 1;
	"./$sort" "$delay" "$number" halfreverse;
	sleep 1;
	"./$sort" "$delay" "$number" reverseevens;
	sleep 1;
	"./$sort" "$delay" "$number" shuffleodds;
	sleep 1;
	"./$sort" "$delay" "$number" circlepass;
	sleep 1;
	"./$sort" "$delay" "$number" sortedheapinput;
fi