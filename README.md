# Command Line sorting visualizer

Sort for fun, sort of fun

The classic bleep-bloop thing in computer science.

![demo_gif_1](https://user-images.githubusercontent.com/43104884/178152823-8c1da8e6-aeb8-44b2-aca5-4c283369a129.gif)<br>
*(A visualization of Quick Sort)*

## A short description...

When started the applications presents an array of numbers. Each integer is represented as a vertical bar to a certain height; larger numbers produces taller heights.

At the beginning, the numbers will be shuffled using a user-chosen hardcoded shuffling method. This will cause the "bars" to be out of order.

Then the sorting algorithm will be started to sort the "bars" into their respective places.

The number of comparisons and writes will be kept tracked of during the sorting process.

## Contents

1. [Pre-compiled Binaries](#pre-compiled-binaries)
    1. [Non-codesigned / Non-notarized executables (+ some lowkey rant)](#non-codesigned--non-notarized-executables--some-lowkey-rant)
    2. [Getting around gatekeeping systems](#getting-around-gatekeeping-systems)
        1. [Windows](#windows)
        2. [macOS](#macos)
2. [Building / Compiling from source](#building--compiling-from-source)
    1. [Prerequisites](#prerequisites)
    2. [GCC](#1-gcc)
        1. [Windows](#windows-1)
        2. [macOS](#macos-1)
        3. [Linux](#linux)
    3. [PortAudio](#2-portaudio)
        1. [Extra preparations for Windows (Installing MSYS2)](#extra-preparations-for-windows-installing-msys2)
        2. [Extra preparations for Linux systems (Installing ASLA)](#extra-preparations-for-linux-systems-installing-asla)
            1. [Using provided package manager](#using-provided-package-manager)
            2. [Install from website](#install-from-website)
        3. [Building PortAudio](#building-portaudio)
        4. [Testing PortAudio](#testing-portaudio)
        5. [Dynamic library on macOS](#dynamic-library-on-macos)
    4. [Building this project](#building-this-project)
        1. [Hooking up PortAudio](#hooking-up-portaudio)
        2. [Build instructions](#build-instructions)
3. [Tweaking](#tweaking)

## Pre-compiled Binaries

Binaries are the stuff you can directly run on the system (example `.exe` on Windows). For some operating systems, the pre-compiled executables are available to download and ready to run. 

The applications can be downloaded from the [releases page](https://github.com/snqzspg/cli-sort-visual/releases).

If the latest release does not contain a download that can be run on your system, you will have to build from the source code by [following the instructions below](#building--compiling-from-source).

### Non-codesigned / Non-notarized executables (+ some lowkey rant)

For small college-student style projects like this, it is not worth it to pay companies like Microsoft and Apple annual fees to codesign or notarize the executables. However, the operating systems will by default block these applications from running, and hints to users to make an association of those application with a possible malware.

During the signing process, the companies will check the application through (presumably) an automated process for any suspicious behaviours. Then a certificate will be given to bundle with the application.

While this is a very important protection system, **charging** for access for these services will hurt small hobbyist projects like this as people ended up thinking that the application is malicious. (I guess it's a fair sacrifice for a greater good in society)

### Getting around gatekeeping systems

**NOTE: Always make sure that you trust the executables before attempting to bypass the gatekeeping systems.**

If you downloaded the binaries from this repository on github, rest assured that the function of the binaries is exactly the same function described by the source code. If this project binaries is found somewhere else, make sure you verify the hashes on the releases pages (Steps are there too!) before opening them. Despite the assurance, the only way for an average user to know if the binaries are trustable is to put on faith on the creator. If you would rather compile from the source code you can do so [in the next section](#building--compiling-from-source). 

### Windows

To bypass you need to click on the link on the dialog that says something on the line of "More info" and click "Run Anyways".

### macOS

To bypass first you need to locate the executable on Finder, then "right click" and select "Open". A dialog box will appear and the option to run the app should be there. It is recommended to close the application afterwards and re-run the application from the Terminal. 

![gatekeeper-guide](https://user-images.githubusercontent.com/43104884/158795352-c50697a7-ee68-48be-b9e5-63409f9f631d.png)

During this process a sounds folder and three `.txt` files, `chord_progression.txt`, `settings.txt` and `sound_frequencies.txt`, may appear in your Home folder. You can delete those once you've unblocked the executables. 

Another way to enable the option to run is to

1. Attempt to run the executable first
2. Go to Settings and then Security & Privacy
3. The text `"<sort_name>" was blocked from use because it is not from an identified developer` should appear.
4. Click on the lock on the bottom left corner, unlock the setting and then click 'Open Anyway'
5. Re-run the executable. The option to run it will appear on the dialog box.

## Building / Compiling from source

In this section are the instructions to produce the executable binaries from the source code.

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

If you want to build a mute version withoud sound, you can [skip ahead to building this project](#building-this-project). If you want a sound (or musical) version, you have to prepare PortAudio.

#### Extra preparations for Windows (Installing MSYS2)

For Windows, you will need to install MSYS to build PortAudio. MSYS2 is a bash terminal that allows one to use bash scripts to configure their makefiles. You can download MSYS2 from [MSYS2](https://www.msys2.org/) website and follow the instructions. 

After installing, you will need to install either `mingw-w64-x86_64-toolchain` (64-bit) or `mingw-w64-i686-toolchain`(32-bit) using `pacman`

Once everything is installed and updated, you can go ahead and [build PortAudio](#building-portaudio), using MSYS2 MINGWXX (**NOT MSYS**) terminal instead of your typical CMD.

#### Extra preparations for Linux systems (Installing ASLA)

*Note: If you already have ALSA or `libasound` on your system, you can [skip this extra preparation](#building-portaudio).*

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

to configure the build files. The above command produces a dynamic library at the end, which on some systems you will have to manually copy the library file to certain locations (NOTE for Windows the Makefile is configured to automatically do this for you, but for macOS, [more on this below](#dynamic-library-on-macos)). For a static configuration (you may not need to copy the libraries, but you will have to copy part of a script):

```
./configure --enable-static --disable-shared
```

*([Optional] For Windows, you can build using directsound using `./configure --with-winapi=directx`; further build instructions can be found on [this page](https://github.com/PortAudio/portaudio/wiki/Notes_about_building_PortAudio_with_MinGW).)*

After the process completes, you can simply type

```
make
```

to build PortAudio. The `bin` and `lib` folders will be created. (Enter `make clean` to undo.)

*Note: For macOS users, if you encountered errors while building (that stops you from building PortAudio), you can try opening up the PortAudio's `Makefile` (**not this project's**) with a text editor, find `-Werror` and delete them. Then save the `Makefile` and run `make` again.*

*Note: For linux users, if you encountered errors while building (that stops you from building PortAudio), if you see something along the lines of "recompile with -fPIC" in your error, you can try to re-configure PortAudio to use static linking instead (`./configure --enable-static --disable-shared`). You will then ensure that you adjust this project's `Makefile` according to [these instructions](#hooking-up-portaudio)*

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

### Building this project

Finally, after getting the required tools and libraries, you can now build the visualizer.

#### Hooking up PortAudio

Firstly, if you have successfully built PortAudio, you can copy the `include` and the `lib` folder to the `src/pa` folder.

If you want to statically link PortAudio to the final executables (no extra dlls required in the final executables), you have to modify the project `Makefile`. If you use dynamic linking (a.k.a. the default setting) you can just jump ahead to [Build instructions](#build-instructions).

For static linking,
1. Open both the project `Makefile` and the PortAudio's `Makefile` with a text editor.
2. Find the line in the PortAudio's `Makefile` that begins with `LIBS = `
   ![locate_linker_codes](https://user-images.githubusercontent.com/43104884/178150408-914ab41e-a555-43d5-b039-8779bbc51ad6.png)
3. Copy everything after the equal sign (Note that the screenshot above is for an Arch Linux system. The code you have to copy will differ for Windows and macOS, but the location will remain the same.)
4. In the project's `Makefile`, find the line that begins with `W_SOUND_LINKERS = `
   ![paste_behind](https://user-images.githubusercontent.com/43104884/178150572-661600f3-717e-4623-b454-ac1116391e7e.png)
5. Paste the copied code at the end of the line. Ensure that there is a space separating the existing code with your pasted one.
   ![pasted_code](https://user-images.githubusercontent.com/43104884/178150628-35e85b3b-95a0-455f-98de-d882908a58d4.png)
6. If there is `-lm` within the pasted code, remove `-lm`.
   ![delete_lm](https://user-images.githubusercontent.com/43104884/178150584-06decca0-cc2c-4f6d-8b95-fccc108ae85c.png)
7. [Windows MSYS2 Only] Find **two** sections of code that are encased with the folloing comments:

```
# NOTE For MSYS2 comment out between here...
<a bunch of code here>
# ...and here.
```
[Windows MSYS2 Only] Comment them out by adding a pound / hash `#` sign at the front of every line of code in between the aformentioned comments. (And yes you have to build this project with MSYS2 terminal too!)

#### Build instructions

You can download (clone) this repository by download the zip and extracting into a folder, or using

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

to build a mute version. (For Windows with MinGW64, under default settings / not statically linked, replace `make` with `mingw32-make` or `mingw64-make`, or drag the `mingw32-make.exe` file into the console).

You can simply type `make` to get some information about the installation process, including how to clean up.

This project does not leave any remnants on your system (less the dependencies). You can delete the entire project folder and it's gone.

To run the binaries, navigate up to the parent of the source folder and run the executable labelled with the sort names.

## Tweaking

If you type `./<sort_name> help` (or `<sort_name>.exe help`, replacing `<sort_name>` with any sort), you can get a somewhat comprehensive guide on how to adjust certain parameters. Note that except for the first option, you cannot skip any option when typing in the parameters. (Use default values if you don't want to change certain values, but want to do so for the options afterwards.)

When you first run any sorting executables, it will generate a `settings.txt` file. Inside there are more settings that you can tweak to modify the behaviour of the visualizer.
