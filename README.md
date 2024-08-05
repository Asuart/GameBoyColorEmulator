# Game Boy Color Emulator
An attemp to implement GameBoy Color emulator in C++. At the moment it's somewhere inbetween GB and GBC.

Emulation is not accurate, but basic features of CPU, PPU, Timer, DMA and MMC are implemented. Audio is not implemented.

Can run some roms, but encounter some strange bugs. More information below.

Allows you to use one save state per rom file, but reloading is unstable. 

## Dependencies
Premake is used for generation project files.

[PixieUI](https://github.com/Asuart/PixieUI) is required to create user interface.

GLFW3 is used to create windows.

GLAD is used to initialize OpenGL.

GLAD and GLFW files are provided with PixieUI.

## Build
Build is configured and tested for Visual Studio 2022 on Windows 10 x64.

Use `git clone --recurse-submodules` to clone repository and initialize it's submodules.

Run `scripts/Setup-Windows.bat` to generate Visual Studio Solution.

## Test results
Blargg's cpu instructions: passed.

![blargg_cpu_instr_result](/assets/blargg_cpu_instr.png)

Blargg's instruction timing: passed (somehow).

![blargg_cpu_instr_result](/assets/blargg_instr_timing.png)

Undefined CPU or MMC error causes bug in Dr. Mario - pill get overlapped. 

## License
- UNLICENSE for this repository (see `UNLICENSE.txt` for more details)
- Premake is licensed under BSD 3-Clause (see included LICENSE.txt file for more details)