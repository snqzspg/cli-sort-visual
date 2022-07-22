@echo off

echo.
echo NOTE: AVOID USING THIS SCRIPT TO BUILD YOUR SORTS!
echo To build now run make and follow the instructions there!
echo This script may not be updated in the future!

set gcc_cmd=gcc.exe

echo.

if not exist "pa\lib\.libs\libportaudio.dll.a" goto NO_LIBPA

for /f "usebackq delims=" %%i in (build_list.txt) do (
    echo Building %%i.c
    "%gcc_cmd%" -Wall -O2 -DDEFAULT_DIS_DELAY=10 -DACCOUNT_PRINT_LEN -DPRINT_TIME_W_O_DELAY=0 -DPA_INSTALLED -DVERSION="""1.1""" -DBUILD="""202207200235""" -DIS_SNAPSHOT=1 terminal_visualiser.c helper/precise_and_acc_time.c helper/beep_log.c helper/sound_player.c helper/anti_quicksort.c helper/ioext.c helper/triangular_input.c helper/smooth_heapify.c "%%i.c" -o "..\%%i" -L./pa/lib/.libs -I./pa/include -lportaudio
    @REM "%gcc_cmd%" -Wall -O2 terminal_visualiser.c "%%i.c" -o "..\%%i"
)
mkdir ..\sounds
"%gcc_cmd%" -Wall -O2 make_sound.c -o ..\sounds\make_wav

copy pa\lib\.libs\libportaudio-2.dll ..

goto END
:NO_LIBPA
echo PortAudio library is not detected.
echo PortAudio library is required for the visualisers to play sound live, and since I cannot package the works of others to my project, the library has to be downloaded seperately.
echo Unfortunately for Windows building it is not as straightforward as macOS or Linux :^(
echo To download 
echo   1 - Go to http://portaudio.com/docs/v19-doxydocs/tutorial_start.html and choose your method of building.
echo       If you want to use MingW64 you can go to https://github.com/PortAudio/portaudio/wiki/Notes_about_building_PortAudio_with_MinGW
echo       You will also need to install MSys
echo   2 - Go to http://www.portaudio.com and click on the \"Download\" button on the left (Hopefully it\'s still there!)
echo   3 - Click on the stable tgz link and download the file.
echo   4 - Extract the contents to a folder.
echo   6 - Follow the build instructions on the website in step 1.
echo   7 - Copy the lib folder and the include folder into the pa folder here.
echo   8 - Re-run this script and see if it works!

:END