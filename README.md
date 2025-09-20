# Median XL Offline Tools

Character editor (to some extent) and item manager for Diablo 2 - Median XL mod. Written in C++ using [Qt framework](https://qt.io/).

## Median XL

- the latest version: https://www.median-xl.com/
- the classic one by BrotherLaz: https://modsbylaz.vn.cz/welcome.html

## Code

**DISCLAIMER**: this is far from a great example of writing code and application architecture!

- current code should be compatible with C++03 / C++98 standard
- release binaries are built against Qt 4.8.7 at the moment
- the latest code is compatible only with the latest mod version

## Building

It can be built basically for any platform. You will need:

1. C++ compiler: msvc, clang, gcc etc.
2. Qt 4 or 5 built with/for your compiler (e.g. Qt 5 from the Qt Online Installer). Building has only been tested against the latest versions - 4.8.7 and 5.15.x.
3. Optional: [CMake](https://cmake.org/) 3.18+, for macOS it's 3.20+

### IDE

- any IDE supporting CMake: open `CMakeLists.txt` or the repo root directory
- Qt Creator: open `MedianXLOfflineTools.pro` (or `CMakeLists.txt`)
- Xcode (macOS): invoke `cmake` on the command line with `-G Xcode`

### Command line

Assuming:
1. your CWD is some build dir, not necessarily inside the repo
2. you have shell variable `qtDir` set to the Qt root directory
3. you have shell variable `repoDir` set to the repo directory

The examples below are written for sh-like shell (Bash, Fish etc.), but will probably also work for Powershell. For CMD shell please adjust accordingly.

#### CMake

1. Configure: `cmake -S "$repoDir" -B . -D "CMAKE_PREFIX_PATH=$qtDir" <other cmake params>`
2. Build: `cmake --build .`

#### QMake

1. Configure: `"$qtDir/bin/qmake" "$repoDir/MedianXLOfflineTools.pro"`
2. Build: run `make` / `jom` / `nmake` / `mingw32-make` depending on your environment

### Example

To generate Visual Studio project that uses x64 build tools (as most likely you're running Windows 64-bit) with Qt built for x86 (32-bit), invoke `cmake` on the command line with `-A Win32 -D "CMAKE_GENERATOR_TOOLSET=host=x64"`
