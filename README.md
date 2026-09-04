<div align="center">

# Dungeon

[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus)](https://en.cppreference.com/w/cpp/17)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform: Linux](https://img.shields.io/badge/platform-Linux-lightgrey?logo=linux\&logoColor=white)](#building)
[![Windows (MinGW)](https://img.shields.io/badge/platform-Windows-lightgrey?logo=mingww64\&logoColor=white)](#building)
[![CMake](https://img.shields.io/badge/CMake-build-064F8C?logo=cmake)](https://cmake.org/)
[![SDL2](https://img.shields.io/badge/SDL2-1A1A2E?logo=sdl\&logoColor=white)](https://www.libsdl.org/)

[![Debian 12 (Bookworm)](https://github.com/rebootless/dungeon/actions/workflows/build-debian-12.yml/badge.svg)](https://github.com/rebootless/dungeon/actions/workflows/build-debian-12.yml)
[![Debian 13 (Trixie)](https://github.com/rebootless/dungeon/actions/workflows/build-debian-13.yml/badge.svg)](https://github.com/rebootless/dungeon/actions/workflows/build-debian-13.yml)
[![Ubuntu 22.04 (Jammy)](https://github.com/rebootless/dungeon/actions/workflows/build-ubuntu-22.04.yml/badge.svg)](https://github.com/rebootless/dungeon/actions/workflows/build-ubuntu-22.04.yml)
[![Ubuntu 24.04 (Noble)](https://github.com/rebootless/dungeon/actions/workflows/build-ubuntu-24.04.yml/badge.svg)](https://github.com/rebootless/dungeon/actions/workflows/build-ubuntu-24.04.yml)
[![Windows (MinGW cross-compile)](https://github.com/rebootless/dungeon/actions/workflows/build-windows-mingw.yml/badge.svg)](https://github.com/rebootless/dungeon/actions/workflows/build-windows-mingw.yml)

**A tile-based dungeon crawler with a built-in level editor**

</div>

Dungeon is a tile-based dungeon crawler written in C++17 using SDL2.

The project combines the game and a built-in level editor into a single application, allowing levels to be created and tested without leaving the game.

## Features

* Tile-based dungeon exploration and gameplay
* Built-in level editor
* Interactive objects and environmental elements
* Multiple dungeon locations and floors
* Configurable game settings
* Integrated developer console
* Cross-platform support for Linux and Windows
* JSON-based level data

## Building

The project uses CMake as its build system and supports building on Linux as well as cross-compiling for Windows using MinGW.

See the project build scripts and repository documentation for platform-specific build instructions.

## Assets

Information about third-party assets and their licenses can be found in [CREDITS.md](CREDITS.md).

## License

This project is licensed under the **GNU General Public License v3.0** — see the [LICENSE](LICENSE) file for details.
