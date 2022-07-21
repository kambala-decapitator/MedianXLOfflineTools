# Median XL Offline Tools

Character editor (to some extent) and item manager for Diablo 2 - Median XL mod. Written on C++ using [Qt framework](https://qt.io/).

## Median XL

- the latest version: https://www.median-xl.com/
- the classic one by BrotherLaz: https://modsbylaz.vn.cz/welcome.html

## Code

**DISCLAIMER**: this is far from a great example of writing code and application architecture!

- current code should be compatible with C++03 standard
- release binaries are built against Qt 4.8.7 at the moment
- the latest code is compatible only with the latest mod version

## Building

It can be built basically for any platform. You will need:

1. C++ compiler
2. Qt 4 or 5

To build from GUI:

- any OS: open `MedianXLOfflineTools.pro` in Qt Creator
- Windows + Qt 4 only: open `MedianXLOfflineTools.sln` in Microsoft Visual Studio 2019, you will also need to install _Qt VS Tools_ extension

For command-line builds you will be using `qmake` and `make` / `jom` / `nmake`, please refer to qmake manual for details.
