#!/usr/bin/env bash

echo 
echo NOTE: AVOID USING THIS SCRIPT TO BUILD YOUR SORTS!
echo To build now run make and follow the instructions there!
echo This script may not be updated in the future!

gcc_cmd=gcc

while read i; do
	echo Building $i.c;
	# "$gcc_cmd" -Wall -O2 -DUSE_ANSI_CLEAR terminal_visualiser.c "$i.c" -o "../$i" -lm;
	"$gcc_cmd" -Wall -O2 -DNO_NCURSES -DVERSION=\"1.0.0\" -DBUILD=\"202207060418\" -DIS_SNAPSHOT=1 terminal_visualiser.c helper/precise_and_acc_time.c helper/beep_log.c helper/anti_quicksort.c helper/ioext.c "$i.c" -o "../$i" -lm;
done < "build_list.txt";
mkdir -p ../sounds;
"$gcc_cmd" -Wall -O2 make_sound.c -o ../sounds/make_wav -lm;
