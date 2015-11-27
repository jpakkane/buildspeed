# Sample project for build speed comparison

This project contains a simple Qt5 application meant for
testing compile speed.

The code was originally developed by Canonical as a network
debugging tool. However in this project we only care about
build performance, not what the application itself is doing.

The original code can be found here:

https://code.launchpad.net/~jpakkane/+junk/netdumpper

## How to compile

Meson

    mkdir buildmeson
    cd buildmeson
    meson ..
    time ninja

Meson unity build

    mkdir buildmu
    cd buildmu
    meson --unity --disable-pch ..
    time ninja

CMake

    mkdir builcmake
    cd buildcmake
    cmake -DCMAKE_BUILD_TYPE=debug -G Ninja ..
    time ninja

## Build times on Raspberry Pi 2

    CMake               2m 20s
    Meson               1m 36s
    Meson unity build      39s

