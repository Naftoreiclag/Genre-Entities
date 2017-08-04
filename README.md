# Genre Entity System

This README is a work-in-progress. Insert short tagline or description here.

## Description

This is Genre Entity System.

### License

Genre Entity System source code, which is located in the `src/` directory,
is licensed under the 
[Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0).
A copy of this license is available in the `LICENSE` file located in the
root of this repository.

Files found outside the `src/` repository, including but not limited to those
located in the `cmake/` directory, may be subject to different licensing
terms. Please see individual files for details.

## Building & Running

Genre Entity System uses [CMake](https://cmake.org/) for build configurations.

### Dependencies

Genre Entity System requires the following libraries to be built correctly.

- [LuaJIT](http://luajit.org/)
  for scripting
- [Easylogging++](https://github.com/muflihun/easyloggingpp/)
  for logging
- [bgfx](https://github.com/bkaradzic/bgfx/)
  for rendering
- [SDL2](https://www.libsdl.org/)
  for windowing, input

*Each library listed above is the copyright of its respective author(s). Please
see individual library homepages for more accurate licensing information.*

## Miscellaneous

### Utilities

In the `tool/` directory are Python scripts that you may find useful:
- `GenerateEngineSrcList.py` populates the cmake file 
  `cmake/EngineSrcList.cmake` from the contents of the `src/` directory.
- `SyncEngineCodelite.py` populates a [Codelite](https://codelite.org/)
  project file (by default, `ide/Codelite/Codelite.project`).

