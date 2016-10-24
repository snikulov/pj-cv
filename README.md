# README #


### What is this repository for? ###

Repository for side project for PJ CV Quality control.

### How do I get set up? ###

**Requirements**

* FFMPEG for H.264 capture from IP Camera (https://github.com/FFmpeg/FFmpeg.git, for Windows use prebuilt binaries here https://ffmpeg.zeranoe.com/builds/)
* Boost C++ Libraries (1.60 or greater http://www.boost.org/users/download/)
* OpenCV C++ Libraries (3.1 https://github.com/opencv/opencv.git)
* C++ compiler which is support C++11 standard (Windows - MSVS Community 2015+, MinGW GCC 4.8+, Linux GCC 4.8+)
* CMake 3.1+


**Preparing environment for Linux (I've used CentOS 7.3 - 64 bit)**


```
#!bash

git clone https://github.com/FFmpeg/FFmpeg.git
cd FFmpeg
./configure --enable-gpl --enable-shared --enable-nonfree --prefix=/usr/local
make; sudo make install
sudo ldconfig /usr/local/lib

```

**Download, unpack Boost C++, and build it with commands**

```
#!bash

./bootstrap.sh
./b2 --build-type=complete --prefix=/usr/local
```


**Install OpenCV**

```
#!bash

git clone https://github.com/opencv/opencv.git
cd opencv; git checkout 3.1.0
mkdir build; cd build; cmake .. -DWITH_GSTREAMER=OFF; make; sudo make install
sudo ldconfig /usr/local/lib64/gcc
```


You all set to build capture!!!

**Preparing environment for Windows (I've used Windows 10 - 64 bit)**

For Visual Studio 2015


```
#!cmd

Download from https://ffmpeg.zeranoe.com/builds/ Release 3.1.4, 64 bit, Shared and Dev archives
Unpack it to same folder 
Should be following structure
C:\FFMPEG\bin -> here are .dll files
C:\FFMPEG\include -> here are .h files
C:\FFMPEG\lib -> here are .a files and .lib

set ENV VARS
set INCLUDE=C:\FFMPEG\include;%INCLUDE%
set LIB=C:\FFMPEG\lib;%LIB%
set PATH=C:\FFMPEG\bin;%PATH%

Download and install Boost C++ release archive
.\bootstrap.cmd
.\b2 --build-type=complete --toolset=msvc-14.0 address-model=64 --prefix=C:\boost -j4 install
set PATH=C:\boost\lib;%PATH%

Install OpenCV
git clone https://github.com/opencv/opencv.git                    _
cd opencv; git checkout 3.1.0
mkdir build
cd build
cmake .. -G"Visual Studio 14 2015 Win64" -DCMAKE_INSTALL_PREFIX=C:\OpenCV
cmake --build . --config Release
cmake --build . --config Release --target INSTALL

set PATH=C:\opencv\x64\vc14\bin\;%PATH%
set OpenCV_DIR=C:\opencv
```


Done!


BUILDING THE CAPTURE


```
#!cmd

git clone git@bitbucket.org:snikulov/pj-cv.git --recurse-submodules
cd pj-cv\cpp\capture\build
cmake .. -G"Visual Studio 14 2015 Win64"
cmake --build . --config Release
```