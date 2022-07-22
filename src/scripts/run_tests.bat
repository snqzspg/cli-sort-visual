@echo off

set sort=%1
if "%sort%"=="" goto END
set delay=5
if not [%2]==[] set delay=%2
set number=512
if not [%3]==[] set number=%3

"%sort%" "%delay%" "%number%" random
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" reverse
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" noshuffle
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" slightshuffle
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" shuffletail
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" shufflehead
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" mergepass
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" quarters
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" reversemergepass
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" mountain
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" radixpass
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" binarysearchtree
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" heapified
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" smoothheapified
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" sortedheapinput
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" halfreverse
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" reverseevens
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" shuffleodds
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" crossweave1
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" circlepass
timeout /t 1 > NUL
"%sort%" "%delay%" "%number%" triangular

:END
