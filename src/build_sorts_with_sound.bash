#!/usr/bin/env bash

echo 
echo NOTE: AVOID USING THIS SCRIPT TO BUILD YOUR SORTS!
echo To build now run make and follow the instructions there!
echo This script may not be updated in the future!

gcc_cmd=gcc

if [ -f "pa/lib/.libs/libportaudio.a" ];
then
	while read i; do
		echo Building $i.c;
		# "$gcc_cmd" -Wall -O2 -DUSE_ANSI_CLEAR terminal_visualiser.c "$i.c" -o "../$i" -lm;
		"$gcc_cmd" -Wall -O2 -DNO_NCURSES -DPA_INSTALLED -DVERSION=\"1.1\" -DBUILD=\"202207180145\" -DIS_SNAPSHOT=1 terminal_visualiser.c helper/precise_and_acc_time.c helper/beep_log.c helper/sound_player.c helper/anti_quicksort.c helper/ioext.c helper/triangular_input.c "$i.c" -o "../$i" -L./pa/lib/.libs -I./pa/include -lm -lportaudio;
	done < "build_list.txt";
	mkdir -p ../sounds;
	"$gcc_cmd" -Wall -O2 make_sound.c -o ../sounds/make_wav -lm;
	# cp lib/.libs/libportaudio.a ..
else
	echo PortAudio library is not detected.
	echo PortAudio library is required for the visualisers to play sound live, and since I cannot package the works of others to my project, the library has to be downloaded seperately.
	echo To download 
	echo   1 - Go to http://www.portaudio.com
	echo   2 - Click on the \"Download\" button on the left \(Hopefully it\'s still there!\)
	echo   3 - Click on the stable tgz link and download the file.
	echo   4 - Extract the contents to a folder.
	echo   5 - Open Terminal / Console, and navigate to the folder using cd.
	echo   6 - Follow the build instructions on the website. 
	echo       For macOS http://files.portaudio.com/docs/v19-doxydocs/compile_mac_coreaudio.html
	echo       For Linux http://files.portaudio.com/docs/v19-doxydocs/compile_linux.html
	echo   7 - Copy the lib folder and the include folder into the pa folder here.
	echo   8 - Re-run this script and see if it works!
	echo   \[Note that for macOS you may need to copy lib/.lib/libportaudio.2.dylib file into your /usr/local/lib/ folder\]
	echo   \[Run \"sudo cp pa/lib/.lib/libportaudio.2.dylib /usr/local/lib/\" to do that\]
	echo   \[To delete run \"sudo mv /usr/local/lib/libportaudio.2.dylib .\" and delete the .dylib file that appeared.\]
	echo   \[Note that for Linux you may need to install libportaudio2 package to get the programs to work. E.g. for debian: sudo apt-get install libportaudio2\]
fi
