# Command Line sorting visualizer

Sort for fun, sort of fun

> This project is on its first days on GitHub. It may take some time to create this page.

## A short description...

When started the applications presents an array of numbers. Each integer is represented as a vertical bar to a certain height; larger numbers produces taller heights.

At the beginning, the numbers will be shuffled using a user-chosen hardcoded shuffling method. This will cause the "bars" to be out of order.

Then the sorting algorithm will be started to sort the "bars" into their respective places.

The number of comparisons and writes will be kept tracked of during the sorting process.

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

PortAudio is a library that is used to play sounds on different Operating Systems.

#### Windows

For Windows, you will need to install MSYS. Download from [MSYS2](https://www.msys2.org/) website and follow the instructions. 

You will need to install either `mingw-w64-x86_64-toolchain` (64-bit) or `mingw-w64-i686-toolchain`(32-bit) using pacman

For PortAudio, download the source code from [PortAudio website](http://www.portaudio.com/) or [PortAudio GitHub](https://github.com/PortAudio/portaudio), then follow the instructions on [this page](https://github.com/PortAudio/portaudio/wiki/Notes_about_building_PortAudio_with_MinGW).

#### macOS

You can download the source code from [PortAudio website](http://www.portaudio.com/) or [PortAudio GitHub](https://github.com/PortAudio/portaudio).

For simple building, you can open the extracted folder in a terminal and enter

```
./configure && make
```

to start building. This creates a file called `libportaudio.2.dylib` inside the hidden folder `lib/.libs/`. You will need to copy the file to the `/usr/local/lib/` folder. You can use the command below to do so:

```
sudo cp lib/.libs/libportaudio.2.dylib /usr/local/lib
```

and the command below to undo it (plus deleting it from the desktop):

```
sudo mv /usr/local/lib/libportaudio.2.dylib ~/Desktop
```

#### Linux

As of the making of this Markdown file (5th July 2022) the [portaudio site](http://www.portaudio.com/) is down. The only page available is [https://github.com/PortAudio/portaudio/wiki/Platforms_Linux](https://github.com/PortAudio/portaudio/wiki/Platforms_Linux).

The site that has the guide to compiling PortAudio for Linux systems is [here](http://portaudio.com/docs/v19-doxydocs/compile_linux.html).

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

*The rest is to be written...*
