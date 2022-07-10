# Command Line sorting visualizer

Sort for fun, sort of fun

## A short description...

When started the applications presents an array of numbers. Each integer is represented as a vertical bar to a certain height; larger numbers produces taller heights.

At the beginning, the numbers will be shuffled using a user-chosen hardcoded shuffling method. This will cause the "bars" to be out of order.

Then the sorting algorithm will be started to sort the "bars" into their respective places.

The number of comparisons and writes will be kept tracked of during the sorting process.

## Contents

1. [Building (or "Installing")](#a-short-description)
    1. [Prerequisites](#prerequisites)
    2. [GCC](#1-gcc)
        1. [Windows](#windows)
        2. [macOS](#macos)
        3. [Linux](#linux)
    3. [PortAudio](#2-portaudio)
        1. [Extra preparations for Windows (Installing MSYS2)](#extra-preparations-for-windows-installing-msys2)
        2. [Extra preparations for Linux systems (Installing ASLA)](#extra-preparations-for-linux-systems-installing-asla)
            1. [Using provided package manager](#using-provided-package-manager)
            2. [Install from website](#install-from-website)
        3. [Building PortAudio](#building-portaudio)
        4. [Testing PortAudio](#testing-portaudio)
        5. [Dynamic library on macOS](#dynamic-library-on-macos)
    4. [Building](#building)
2. [Tweaking](#tweaking)

## Building (or "Installing")

Currently there are no binaries available to download, so you may have to do some code compiling yourself.

### Prerequisites

To compile this project you will need two items:

1. GNU Compiler Collection (GCC)
2. PortAudio (This can be omitted if you want to build a mute version)

### 1. GCC

This project is designed to be compiled using the GNU Compiler Collection (GCC).

#### Windows

For windows, it is recommended to use [MinGW64](https://www.mingw-w64.org/) (a fork of the original MinGW, standing for Minimalist GNU for Windows) to compile the code.

You can download the installers / zip packages via the [download link on the MinGW64 website](https://www.mingw-w64.org/downloads/#mingw-builds) or on their [SourceForge](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/).

For your convenience you can add the bin folder of your MinGW64 installation to your Environment Variables (You can search on how to do this)

1. Right click on start menu button and click System (or My Computer and click `properties`).
2. Inside the system dialog find the link that says `Advanced System Settings` and then click on it.
3. Inside the advanced settings click on the `Environment Variables` button. 
4. Find PATH in your users environment variables (the top white box), edit it by adding the bin folder in your MinGW64 installation.

#### macOS

GCC is included in the Xcode Command Line Tools. You can install Xcode Command Tools by entering the following in the Terminal

```
xcode-select --install
```

The command line tools can be uninstalled by deleting the `/Library/Developer/CommandLineTools` folder.

#### Linux

Use your package manager to install GCC and make (`apt install` on debian (Ubuntu) systems, `pacman -S` on Arch systems).

On debian based systems you can install all necessary tools with:

```
sudo apt install build-essential
```

### 2. PortAudio

PortAudio is a library that is used to play sounds on different Operating Systems. It is required to create a version of the visualizer with sound.

#### Extra preparations for Windows (Installing MSYS2)

For Windows, you will need to install MSYS to build PortAudio. MSYS2 is a bash terminal that allows one to use bash scripts to configure their makefiles. You can download MSYS2 from [MSYS2](https://www.msys2.org/) website and follow the instructions. 

After installing, you will need to install either `mingw-w64-x86_64-toolchain` (64-bit) or `mingw-w64-i686-toolchain`(32-bit) using `pacman`

Once everything is installed and updated, you can go ahead and [build PortAudio] (#building-portaudio), using MSYS2 MINGWXX (**NOT MSYS**) terminal instead of your typical CMD.

#### Extra preparations for Linux systems (Installing ASLA)

*Note: If you already have ALSA or `libasound` on your system, you can skip this extra preparation.*

For linux distros, PortAudio strongly recommends installing Advanced Linux Sound Architecture (ALSA) project to interface with the sound system on different distros. There are a couple of ways to do this.

##### Using provided package manager

You can install ALSA library using the command line package manager provided by the distro. The library name could be `libasound-dev`. Example for debian-based systems (including Ubuntu and its derivatives):

```
sudo apt-get install libasound-dev
```

##### Install from website

You can download `alsa-lib` compressed tarball from [ALSA's website](https://www.alsa-project.org/wiki/Main_Page) and extract its contents. Example:

```
tar zxvf alsa-lib-1.2.7.2.tar.bz2
```

Using command line, navigate to the extracted folder (using `cd`) and enter (for default)

```
./configure
```

or (for static linking)

```
./configure --enable-shared=no --enable-static=yes
```

Then enter the following command to install:

```
make install
```

or

```
sudo make install
```

if extra permissions are needed.

#### Building PortAudio

For PortAudio, download the source code from [PortAudio website](http://www.portaudio.com/) or [PortAudio GitHub](https://github.com/PortAudio/portaudio).

Then extract and navigate to the folder of the extracted contents using the bash/zsh command line (for Windows use MSYS2 MINGWXX) (using `cd`). Firstly enter 

```
./configure
```

to configure the build files. The above command produces a dynamic library at the end, which on some systems you will have to manually copy the library file to certain locations (more on this below). For a static configuration (you may not need to copy the libraries, but you will have to copy part of a script):

```
./configure --enable-static --disable-shared
```

*([Optional] For Windows, you can build using directsound using `./configure --with-winapi=directx`; further build instructions can be found on [this page](https://github.com/PortAudio/portaudio/wiki/Notes_about_building_PortAudio_with_MinGW).)*

After the process completes, you can simply type

```
make
```

to build PortAudio. The `bin` and `lib` folders will be created. (Enter `make clean` to undo.)

#### Testing PortAudio

**Lower the volume of your device before trying out the following step.**

To test the sound playing property of your built PortAudio, enter `bin/paex_sine` or `bin/paex_saw`. The command will cause your device to play a sine-wave or a saw-wave sound respectively.

#### Dynamic library on macOS

In your PortAudio folder the building process creates a file called `libportaudio.2.dylib` inside the hidden folder `lib/.libs/`. You will need to copy the file to the `/usr/local/lib/` folder. You can use the command below to do so:

```
sudo cp lib/.libs/libportaudio.2.dylib /usr/local/lib
```

and the command below to undo it (plus deleting it from the desktop):

```
sudo mv /usr/local/lib/libportaudio.2.dylib ~/Desktop
```

### Building

Finally, after getting the required tools and libraries, you can now build the visualizer.

Firstly, if you have successfully built PortAudio, you can copy the `include` and the `lib` folder to the `src/pa` folder.

Then, you can download (clone) this repository by download the zip and extracting into a folder, or using

```
git clone https://github.com/snqzspg/cli-sort-visual makeshift_visualizer
```

if you have Git installed.

To build, you first navigate into the src folder (`cd src`) and then you can simply type

```
make withsound
```

if you managed to have PortAudio, or

```
make nosound
```

to build a mute version. (For Windows with MinGW64 replace `make` with `mingw32-make` or `mingw64-make`, or drag the `mingw32-make.exe` file into the console).

You can simply type `make` to get some information about the installation process, including how to clean up.

This project does not leave any remnants on your system. You can delete the entire project folder and it's gone.

To run the binaries, navigate up to the parent of the source folder and run the executable labelled with the sort names.

## Tweaking

If you type `./<sort_name> help` (or `<sort_name>.exe help`, replacing `<sort_name>` with any sort), you can get a somewhat comprehensive guide on how to adjust certain parameters. Note that except for the first option, you cannot skip any option when typing in the parameters. (Use default values if you don't want to change certain values, but want to do so for the options afterwards.)

When you first run any sorting executables, it will generate a `settings.txt` file. Inside there are more settings that you can tweak to modify the behaviour of the visualizer.
