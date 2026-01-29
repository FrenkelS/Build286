## Build286
![Build286](readme_imgs/build286.png?raw=true)

[Ken Silverman's Build Engine](https://www.advsys.net/ken/build.htm) was originally designed for 32-bit DOS computers.
It's written in C code with assembly code.

Build286 is a port for PCs with a 16-bit processor like a 286, and with VGA or MCGA graphics.
It uses 64 kB of EMS memory, if available, as an upper memory block to store data.

Download Build286 [here](https://github.com/FrenkelS/Build286/releases).

## Command line arguments:
|Command line argument|Effect               |
|---------------------|---------------------|
|\<file\>             |Load custom map      |
|-noems               |Disable EMS          |

## Building:
1) Install [Watcom](https://github.com/open-watcom/open-watcom-v2) on Windows.

2) Run `setenvwc.bat` followed by `bwc16.bat`.

It's possible to build a 32-bit version of Build286 with [DJGPP](https://github.com/andrewwutw/build-djgpp) and Watcom.
First run `setenvdj.bat` once and then `bdj32.bat` for DJGPP, and `setenvwc.bat` followed by `bwc32.bat` for Watcom.
For debugging purposes, the Zone memory can be increased significantly this way.
