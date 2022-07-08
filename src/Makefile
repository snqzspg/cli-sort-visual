CC=gcc
CFLAGS=-Wall -O2
LD=gcc
LDFLAGS=
WIN_INFO=

TERM_VIS_FLAGS = -DDEFAULT_DIS_DELAY=10

LINKERFLAG = -lm

NO_NCURSES_FLAGS = -DNO_NCURSES

W_SOUND_FLAGS = -DPA_INSTALLED
W_SOUND_LINKERS = -L./pa/lib/.libs -I./pa/include -lportaudio

VERSION = 1.0.0
BUILD = 202207081710
IS_SNAPSHOT=1

DVERSION_FLAGS = -DVERSION=\"$(VERSION)\"
DBUILD_FLAGS = -DBUILD=\"$(BUILD)\"
DIS_SNAPSHOT_FLAGS = -DIS_SNAPSHOT=$(IS_SNAPSHOT)

BUILDLIST := heap_sort \
	merge_sort \
	stooge_sort \
	slow_sort \
	bozo_sort \
	better_bozo_sort \
	circle_sort \
	selection_sort \
	bitonic_sort \
	bubble_sort \
	insertion_sort \
	optimised_bubble_sort \
	binary_insertion_sort \
	bogo_sort \
	gnome_sort \
	quick_sort_random_pivot \
	quick_sort \
	quick_sort_gfg \
	quick_sort_john \
	quick_sort_john_random_pivot \
	shell_sort \
	shell_sort_ciura \
	intro_sort \
	intro_sort_gfg \
	intro_bozo_sort \
	intro_sort_john \
	intro_sort_john_random_pivot \
	radix_sort_lsd \
	radix_sort_lsd_alt \
	cocktail_shaker_sort \
	optimised_cocktail_shaker_sort \
	radix_sort_msd \
	bogo_sort_for_show \
	tim_sort \
	tim_sort_3_min_run \
	stalin_sort \
	faulty_tim_sort_1 \
	faulty_tim_sort_2 \
	faulty_tim_sort_3 \
	faulty_tim_sort_4 \
	stalin_sort_one_by_one \
	cycle_sort \
	better_cycle_sort \
	further_optimised_bubble_sort \
	further_optimised_cocktail_shaker_sort

SRCS = $(BUILDLIST:%=%.c)
OBJS = $(BUILDLIST:%=objs/%.o)
BINS = $(BUILDLIST:%=../%)
SOUND_OBJS = $(BUILDLIST:%=objs/%_s.o)
SOUND_BINS = $(BUILDLIST:%=../%_s)
CLEAN_BINS = $(BUILDLIST:%=../%_c)
CLEAN_BIN_OS = $(BUILDLIST:%=objs/%.o_d)
SRCS = $(BUILDLIST:%=%.c)

TERM_VIS_SRC = terminal_visualiser.c
TERM_VIS_OBJ = $(TERM_VIS_SRC:%.c=objs/%.o)
TERM_VIS_OBJ_MUTE = $(TERM_VIS_SRC:%.c=objs/%_mute.o)
CLEAN_TERM_VIS_OBJ = $(TERM_VIS_SRC:%.c=objs/%.o_d)
CLEAN_TERM_VIS_OBJ_MUTE = $(TERM_VIS_SRC:%.c=objs/%_mute.o_d)

MAKE_WAV_SRC = make_sound.c
MAKE_WAV_OBJ = $(MAKE_WAV_SRC:%.c=objs/%.o)
MAKE_WAV_OBJ_CLEAN = $(MAKE_WAV_SRC:%.c=objs/%.o_d)
MAKE_WAV_BIN = ../sounds/make_wav
MAKE_WAV_BIN_CLEAN = $(MAKE_WAV_BIN:%=%_d)

# https://stackoverflow.com/questions/714100/os-detecting-makefile
ifeq ($(OS),Windows_NT)
    TERM_VIS_FLAGS = -DDEFAULT_DIS_DELAY=10 -DACCOUNT_PRINT_LEN -DPRINT_TIME_W_O_DELAY=0

    RM = del
    rmiexist = if exist $(subst /,\,$1) del $(subst /,\,$1)
    RMDIR = rmdir
    mkdir = if not exist $(subst /,\,$1) mkdir $(subst /,\,$1)
    FixPath = $(subst /,\,$1)
    ECHOQUOT = 
    ESCAMP = ^
    ECHONOTHING = .
    PAUSECMD = @pause
    EXEEXT = .exe
    RMOBJDIR = -rmdir /q objs
    RMSNDSDIR = -rmdir /q ..\sounds
    ListAllCFiles = $(subst \,/,$(shell if exist $1 for %%i in ($1\*.c) do echo %%i))
    CPYWINDLL = if not exist ..\libportaudio-2.dll copy pa\lib\.libs\libportaudio-2.dll ..\libportaudio-2.dll
    RUN_TESTS_SCRIPT_FILE = run_tests.bat
    CPY_SCRIPT_FILE = if not exist ..\run_tests.bat copy scripts\run_tests.bat ..\run_tests.bat
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S), Linux)
        RM = rm
        rmiexist = if [ -f $1 ]; then rm $1; fi
        RMDIR = rm -d 
        mkdir = if [ ! -d $1 ]; then mkdir -p $1; fi
        FixPath = $1
        ECHOQUOT = "
        ESCAMP = 
        ECHONOTHING = 
        PAUSECMD = @read -p 'Press <enter> to continue... ' pgans
        EXEEXT = 
        RMOBJDIR = if [ -d objs ] && [ -z "$$(ls -A objs)" ]; then rm -d objs; fi
        RMSNDSDIR = if [ -d ../sounds ] && [ -z "$$(ls -A ../sounds)" ]; then rm -d ../sounds; fi
        ListAllCFiles = $(shell if [ -d $1 ]; then find $1 -type f -name '*.c'; fi)
        CPYWINDLL = 
        RUN_TESTS_SCRIPT_FILE = run_tests.bash
        CPY_SCRIPT_FILE = if [ ! -f ../run_tests.bash ]; then cp scripts/run_tests.bash ../run_tests.bash; fi
    endif
    ifeq ($(UNAME_S),Darwin)
        RM = rm
        rmiexist = if [ -f $1 ]; then rm $1; fi
        RMDIR = rm -d 
        mkdir = if [ ! -d $1 ]; then mkdir -p $1; fi
        FixPath = $1
        ECHOQUOT = "
        ESCAMP = 
        ECHONOTHING = 
        PAUSECMD = @read -p 'Press <enter> to continue... ' pgans
        EXEEXT = 
        RMOBJDIR = if [ -d objs ] && [ -z "$$(ls -A objs)" ]; then rm -d objs; fi
        RMSNDSDIR = if [ -d ../sounds ] && [ -z "$$(ls -A ../sounds)" ]; then rm -d ../sounds; fi
        ListAllCFiles = $(shell if [ -d $1 ]; then find $1 -type f -name '*.c'; fi)
        CPYWINDLL = 
        RUN_TESTS_SCRIPT_FILE = run_tests.bash
        CPY_SCRIPT_FILE = if [ ! -f ../run_tests.bash ]; then cp scripts/run_tests.bash ../run_tests.bash; fi
    endif
endif

HELPER_SRCS = helper/precise_and_acc_time.c \
	helper/beep_log.c \
	helper/anti_quicksort.c \
	helper/ioext.c
HELPER_OBJS = $(HELPER_SRCS:helper/%.c=objs/%.o)
CLEAN_HELPER_OBJS = $(HELPER_SRCS:helper/%.c=objs/%.o_d)

HELPER_SOUND_SRCS = helper/sound_player.c
HELPER_SOUND_OBJS = $(HELPER_SOUND_SRCS:helper/%.c=objs/%.o)
CLEAN_HELPER_SOUND_OBJS = $(HELPER_SOUND_SRCS:helper/%.c=objs/%.o_d)

.PHONY = all version prerequisites to_build cleanup withsound nosound build_done_msg custom_notice copy_binaries_notice custom_help objs $(BINS) clean $(CLEAN_BINS) $(CLEAN_BIN_OS)

all: version prerequisites to_build cleanup
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)Woah that was a lot! If you can't scroll back you can type 'make prerequisites', 'make to_build', 'make cleanup' to display each section individually$(ECHOQUOT)
	@echo $(ECHOQUOT)Enter 'make version' to see version number$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)Anyways have fun with this project!$(ECHOQUOT)
	@echo$(ECHONOTHING)

version:
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)############################################$(ECHOQUOT)
	@echo $(ECHOQUOT)#                                          #$(ECHOQUOT)
	@echo $(ECHOQUOT)#     Snqzs' PG CLI Sorting Visualizer     #$(ECHOQUOT)
	@echo $(ECHOQUOT)#        Sort for fun, sort of fun         #$(ECHOQUOT)
	@echo $(ECHOQUOT)#    Version $(VERSION) (Build $(BUILD))    #$(ECHOQUOT)
	@echo $(ECHOQUOT)#                                          #$(ECHOQUOT)
	@echo $(ECHOQUOT)############################################$(ECHOQUOT)

prerequisites:
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)================$(ECHOQUOT)
	@echo $(ECHOQUOT) PREREQUISITES$(ECHOQUOT)
	@echo $(ECHOQUOT)================$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)This project requires PortAudio library to play sound live. $(ECHOQUOT)
	@echo $(ECHOQUOT)You will have to download and build PortAudio to build with sound.$(ECHOQUOT)
	@echo $(ECHOQUOT)To download: http://files.portaudio.com/download.html$(ECHOQUOT)
	@echo $(ECHOQUOT)HINT: For windows with MinGW64 you will also need to install MSYS.$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)To build PortAudio you can follow the instructions on their website.$(ECHOQUOT)
	@echo $(ECHOQUOT)Then copy the 'include' and 'lib' folder into the 'pa' folder.$(ECHOQUOT)
	@echo $(ECHOQUOT)FOR WINDOWS: You will need to copy your MSYS' 'libgcc_s_dw2-1.dll' to the same location as the binaries if you build with directx.$(ECHOQUOT)
	@echo $(ECHOQUOT)FOR macOS: You may need to copy lib/.lib/libportaudio.2.dylib file into your /usr/local/lib/ folder.$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)I hope that you are able to get PortAudio working, 'cause if not it will be a bit of a bummer!$(ECHOQUOT)
	@echo $(ECHOQUOT)(You'll have no sound nor music to enjoy D:)$(ECHOQUOT)

to_build:
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)================$(ECHOQUOT)
	@echo $(ECHOQUOT) BUILD$(ECHOQUOT)
	@echo $(ECHOQUOT)================$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)Now to get started, you can type one of the following:$(ECHOQUOT)
	@echo $(ECHOQUOT)  1. make nosound (or mingw32-make nosound)$(ECHOQUOT)
	@echo $(ECHOQUOT)  2. make withsound (or mingw32-make withsound) [Requires PortAudio]$(ECHOQUOT)
	@echo $(ECHOQUOT)  3. make nosound_w_custom (also compile customised sources in 'add_sorts' folder)$(ECHOQUOT)
	@echo $(ECHOQUOT)  4. make withsound_w_custom (also compile customised sources in 'add_sorts' folder) [Requires PortAudio]$(ECHOQUOT)

cleanup:
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)Welcome! Before you get started, you need to ensure you have the prerequisites.$(ECHOQUOT)
	@echo $(ECHOQUOT)If you had the prerequisites you can skip directly to BUILD below.$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)NOTE: The binaries will be generated in the parent directory. It is strongly recommended for this 'src' folder to be in a relatively empty folder.$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)================$(ECHOQUOT)
	@echo $(ECHOQUOT) CLEANUP$(ECHOQUOT)
	@echo $(ECHOQUOT)================$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)To clean up the object and binary files, you can type the following commands:$(ECHOQUOT)
	@echo $(ECHOQUOT)  1. make clean (Clean all objs and binaries)$(ECHOQUOT)
	@echo $(ECHOQUOT)  2. make clean_bins (Only cleans generated binary files) [Requires PortAudio]$(ECHOQUOT)
	@echo $(ECHOQUOT)  3. make clean_objs (Only cleans generated object files)$(ECHOQUOT)

withsound: $(SOUND_BINS) build_done_msg custom_notice copy_binaries_notice
	@echo$(ECHONOTHING)

withsound_w_custom : $(SOUND_BINS) $(patsubst add_sorts/%.c,__wsound__add_sorts/%.c,$(call ListAllCFiles,add_sorts)) build_done_msg copy_binaries_notice
	@echo$(ECHONOTHING)

nosound: $(BINS) build_done_msg custom_notice
	@echo$(ECHONOTHING)

nosound_w_custom : $(BINS) $(patsubst add_sorts/%.c,__nosound__add_sorts/%.c,$(call ListAllCFiles,add_sorts)) build_done_msg
	@echo$(ECHONOTHING)

build_done_msg:
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)================$(ECHOQUOT)
	@echo $(ECHOQUOT) BUILD COMPLETE$(ECHOQUOT)
	@echo $(ECHOQUOT)================$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)Now you can navigate up one folder to run the binaries.$(ECHOQUOT)

custom_notice:
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)Note that any custom scripts are not being built.$(ECHOQUOT)
	@echo $(ECHOQUOT)You can enter 'make custom_help' for more information.$(ECHOQUOT)

copy_binaries_notice:
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)FOR WINDOWS: You will need to copy your MSYS' 'libgcc_s_dw2-1.dll' to the same location as the binaries if you built PortAudio with directx.$(ECHOQUOT)
	@echo $(ECHOQUOT)FOR macOS: You may need to copy lib/.lib/libportaudio.2.dylib file into your /usr/local/lib/ folder.$(ECHOQUOT)

custom_help:
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)===============$(ECHOQUOT)
	@echo $(ECHOQUOT) CUSTOMISATION$(ECHOQUOT)
	@echo $(ECHOQUOT)===============$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)All additional scripts that you want to be built along with the existing scripts are to be placed in the 'add_sorts' folder.$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)Ensure that you #include $(ESCAMP)<../terminal_visualiser.h$(ESCAMP)> relative from that folder.$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)After that, you can either type$(ECHOQUOT)
	@echo $(ECHOQUOT)  1. make __wsound__add_sorts/$(ESCAMP)<sort_name$(ESCAMP)>.c (with sound)$(ECHOQUOT)
	@echo $(ECHOQUOT)  2. make __nsound__add_sorts/$(ESCAMP)<sort_name$(ESCAMP)>.c (no PortAudio and sound)$(ECHOQUOT)
	@echo $(ECHOQUOT)to build the file individually, or$(ECHOQUOT)
	@echo $(ECHOQUOT)  1. make withsound_w_custom (with sound)$(ECHOQUOT)
	@echo $(ECHOQUOT)  2. make nosound_w_custom (no PortAudio and sound)$(ECHOQUOT)
	@echo $(ECHOQUOT)to build all of the sorts.$(ECHOQUOT)
	@echo$(ECHONOTHING)

$(BINS): objs sounds $(TERM_VIS_OBJ_MUTE) $(HELPER_OBJS) $(OBJS) $(MAKE_WAV_BIN) ../$(RUN_TESTS_SCRIPT_FILE)
	$(LD) $(LDFLAGS) $(TERM_VIS_OBJ_MUTE) $(HELPER_OBJS) objs/$(@:../%=%.o) -o $@ $(LINKERFLAG)

$(SOUND_BINS): objs sounds $(TERM_VIS_OBJ) $(HELPER_OBJS) $(HELPER_SOUND_OBJS) $(OBJS) libportaudio-2.dll $(MAKE_WAV_BIN) ../$(RUN_TESTS_SCRIPT_FILE)
	$(LD) $(LDFLAGS) $(TERM_VIS_OBJ) $(HELPER_OBJS) $(HELPER_SOUND_OBJS) objs/$(@:../%_s=%.o) -o $(@:%_s=%) $(LINKERFLAG) $(W_SOUND_LINKERS)

__wsound__add_sorts/%.c: objs $(TERM_VIS_OBJ) $(HELPER_OBJS) $(HELPER_SOUND_OBJS) $(patsubst add_sorts/%.c,objs/%.o,$(call ListAllCFiles,add_sorts)) libportaudio-2.dll
	$(LD) $(LDFLAGS) $(TERM_VIS_OBJ) $(HELPER_OBJS) $(HELPER_SOUND_OBJS) objs/$(notdir $(@:__wsound__%.c=%.o)) $(LINKERFLAG) -o ../$(notdir $(@:__wsound__%.c=%)) $(LINKERFLAG) $(W_SOUND_LINKERS)

__nosound__add_sorts/%.c: objs $(TERM_VIS_OBJ_MUTE) $(HELPER_OBJS) $(patsubst add_sorts/%.c,objs/%.o,$(call ListAllCFiles,add_sorts))
	$(LD) $(LDFLAGS) $(TERM_VIS_OBJ_MUTE) $(HELPER_OBJS) objs/$(notdir $(@:__nosound__%.c=%.o)) $(LINKERFLAG) -o ../$(notdir $(@:__nosound__%.c=%)) $(LINKERFLAG)

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -c $(@:objs/%.o=%.c) $(LINKERFLAG) -o $@

$(patsubst add_sorts/%.c,objs/%.o,$(call ListAllCFiles,add_sorts)): $(call ListAllCFiles,add_sorts)
	$(CC) $(CFLAGS) -c $(@:objs/%.o=add_sorts/%.c) $(LINKERFLAG) -o $@

objs:
	$(call mkdir,objs)
	$(call mkdir,add_sorts)

$(TERM_VIS_OBJ): $(TERM_VIS_SRC)
	$(CC) $(CFLAGS) $(TERM_VIS_FLAGS) $(NO_NCURSES_FLAGS) $(W_SOUND_FLAGS) $(DVERSION_FLAGS) $(DBUILD_FLAGS) $(DIS_SNAPSHOT_FLAGS) -c $(@:objs/%.o=%.c) $(LINKERFLAG) $(W_SOUND_LINKERS) -o $@

$(TERM_VIS_OBJ_MUTE): $(TERM_VIS_SRC)
	$(CC) $(CFLAGS) $(TERM_VIS_FLAGS) $(NO_NCURSES_FLAGS) $(DVERSION_FLAGS) $(DBUILD_FLAGS) $(DIS_SNAPSHOT_FLAGS) -c $(@:objs/%_mute.o=%.c) $(LINKERFLAG) -o $@

$(HELPER_OBJS): $(HELPER_SRCS)
	$(CC) $(CFLAGS) $(NO_NCURSES_FLAGS) -c $(@:objs/%.o=helper/%.c) $(LINKERFLAG) -o $@

$(HELPER_SOUND_OBJS): $(HELPER_SOUND_SRCS)
	$(CC) $(CFLAGS) $(NO_NCURSES_FLAGS) -c $(@:objs/%.o=helper/%.c) $(LINKERFLAG) $(W_SOUND_LINKERS) -o $@

libportaudio-2.dll:
	$(CPYWINDLL)

sounds:
	$(call mkdir,../sounds)

$(MAKE_WAV_BIN): $(MAKE_WAV_OBJ)
	$(LD) $(LDFLAGS) $< -o $@ $(LINKERFLAG)

$(MAKE_WAV_OBJ): $(MAKE_WAV_SRC)
	$(CC) $(CFLAGS) -c $< -o $@ $(LINKERFLAG)

../$(RUN_TESTS_SCRIPT_FILE):
	$(CPY_SCRIPT_FILE)

clean: clean_notice clean_all_notice $(CLEAN_BINS) $(patsubst add_sorts/%.c,../%_c,$(call ListAllCFiles,add_sorts)) $(MAKE_WAV_BIN_CLEAN) clean_libportaudio-2.dll clean_$(RUN_TESTS_SCRIPT_FILE) clean_objs_1
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)The generated binary and object files has been successfully cleaned!$(ECHOQUOT)

clean_objs: clean_notice clean_objs_notice clean_objs_1
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)The generated object files has been successfully cleaned!$(ECHOQUOT)

clean_bins: clean_notice clean_bins_notice $(CLEAN_BINS) $(patsubst add_sorts/%.c,../%_c,$(call ListAllCFiles,add_sorts)) $(MAKE_WAV_BIN_CLEAN) clean_libportaudio-2.dll clean_$(RUN_TESTS_SCRIPT_FILE)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)The generated binary files has been successfully cleaned!$(ECHOQUOT)

clean_objs_1: $(CLEAN_BIN_OS) $(patsubst add_sorts/%.c,objs/%.o_d,$(call ListAllCFiles,add_sorts)) $(CLEAN_TERM_VIS_OBJ) $(CLEAN_TERM_VIS_OBJ_MUTE) $(CLEAN_HELPER_OBJS) $(CLEAN_HELPER_SOUND_OBJS) $(MAKE_WAV_OBJ_CLEAN)
	$(RMOBJDIR)

clean_notice:
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)================$(ECHOQUOT)
	@echo $(ECHOQUOT)  CLEANUP   $(ECHOQUOT)
	@echo $(ECHOQUOT)================$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)Note that for some 'make clean's, they will most likely to just delete everything in the bin / objs folders without warning.$(ECHOQUOT)
	@echo $(ECHOQUOT)If you had anything in those folders, make would gleefully delete them (permanently).$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)In this Makefile, only the specified binaries generated by this Makefile will be deleted, and the objs folder will be deleted if empty.$(ECHOQUOT)
	@echo $(ECHOQUOT)Note that any files generated by the applications themselves will not get deleted.$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)It is good practice that you ensure that there is nothing of importance in the parent folder (this project only) as well as the objs folder, for this project and others (with bin folder).$(ECHOQUOT)
	@echo$(ECHONOTHING)

clean_all_notice:
	@echo $(ECHOQUOT)This option will delete all object and binary files.$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)To only delete the object files, enter make clean_objs$(ECHOQUOT)
	@echo $(ECHOQUOT)To only delete the binary files, enter make clean_bins$(ECHOQUOT)
	@echo $(ECHOQUOT)Press $(ESCAMP)<ctrl-c$(ESCAMP)> to cancel$(ECHOQUOT)
	@echo$(ECHONOTHING)
	$(PAUSECMD)

clean_objs_notice:
	@echo $(ECHOQUOT)This option will only delete all object files.$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)To only delete the binary files, enter make clean_bins$(ECHOQUOT)
	@echo $(ECHOQUOT)To delete both the object and the binary files, enter make clean$(ECHOQUOT)
	@echo $(ECHOQUOT)Press $(ESCAMP)<ctrl-c$(ESCAMP)> to cancel$(ECHOQUOT)
	@echo$(ECHONOTHING)
	$(PAUSECMD)

clean_bins_notice:
	@echo $(ECHOQUOT)This option will only delete all binary files.$(ECHOQUOT)
	@echo$(ECHONOTHING)
	@echo $(ECHOQUOT)To only delete the object files, enter make clean_objs$(ECHOQUOT)
	@echo $(ECHOQUOT)To delete both the object and the binary files, enter make clean$(ECHOQUOT)
	@echo $(ECHOQUOT)Press $(ESCAMP)<ctrl-c$(ESCAMP)> to cancel$(ECHOQUOT)
	@echo$(ECHONOTHING)
	$(PAUSECMD)

$(CLEAN_BINS):
	$(call rmiexist,$(@:%_c=%)$(EXEEXT))

$(patsubst add_sorts/%.c,../%_c,$(call ListAllCFiles,add_sorts)):
	$(call rmiexist,$(@:%_c=%)$(EXEEXT))

$(CLEAN_BIN_OS):
	$(call rmiexist,$(@:%_d=%))

$(patsubst add_sorts/%.c,objs/%.o_d,$(call ListAllCFiles,add_sorts)):
	$(call rmiexist,$(@:%_d=%))

$(CLEAN_TERM_VIS_OBJ):
	$(call rmiexist,$(@:%_d=%))

$(CLEAN_TERM_VIS_OBJ_MUTE):
	$(call rmiexist,$(@:%_d=%))

$(CLEAN_HELPER_OBJS):
	$(call rmiexist,$(@:%_d=%))

$(CLEAN_HELPER_SOUND_OBJS):
	$(call rmiexist,$(@:%_d=%))

$(MAKE_WAV_BIN_CLEAN):
	$(call rmiexist,$(@:%_d=%)$(EXEEXT))
	$(RMSNDSDIR)

$(MAKE_WAV_OBJ_CLEAN):
	$(call rmiexist,$(@:%_d=%))

clean_libportaudio-2.dll:
	$(call rmiexist,$(@:clean_%=../%))

clean_$(RUN_TESTS_SCRIPT_FILE):
	$(call rmiexist,$(@:clean_%=../%))