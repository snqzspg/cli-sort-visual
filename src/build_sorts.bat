@echo off

echo.
echo NOTE: AVOID USING THIS SCRIPT TO BUILD YOUR SORTS!
echo To build now run make and follow the instructions there!
echo This script may not be updated in the future!

set gcc_cmd=gcc.exe

echo.

for /f "usebackq delims=" %%i in (build_list.txt) do (
    echo Building %%i.c
    "%gcc_cmd%" -Wall -O2 -DDEFAULT_DIS_DELAY=10 -DACCOUNT_PRINT_LEN -DPRINT_TIME_W_O_DELAY=0 -DVERSION="""1.1""" -DBUILD="""202207131804""" -DIS_SNAPSHOT=1 terminal_visualiser.c helper/precise_and_acc_time.c helper/beep_log.c helper/anti_quicksort.c helper/ioext.c helper/triangular_input.c "%%i.c" -o "..\%%i"
    @REM "%gcc_cmd%" -Wall -O2 terminal_visualiser.c "%%i.c" -o "..\%%i"
)
mkdir ..\sounds
"%gcc_cmd%" -Wall -O2 make_sound.c -o ..\sounds\make_wav