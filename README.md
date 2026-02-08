## Build286
![Build286](readme_imgs/build286.png?raw=true)

[Ken Silverman's Build Engine](https://www.advsys.net/ken/build.htm) was originally designed for 32-bit DOS computers.
It's written in C code with assembly code.

Build286 is a port for PCs with a 16-bit processor like a 286, and with VGA or MCGA graphics.
It uses 64 kB of EMS memory, if available, as an upper memory block to store data.

Download Build286 [here](https://github.com/FrenkelS/Build286/releases).

## Controls:
|Action                           |Keys            |
|---------------------------------|----------------|
|Walk                             |Arrow keys      |
|Run                              |Left Shift      |
|Walk slowly                      |F1, F2, F9, F10 |
|Strafe                           |Right Ctrl      |
|Strafe left and right            |<, >            |
|Strafe left and right slowly     |F3, F4          |
|Jump                             |A               |
|Crouch                           |Z               |
|Look up and down                 |Left Ctrl + A, Z|
|Change brightness                |F11             |
|Change aspect ratio              |9, 0, +, -      |
|Render Build Engine sprite       |1, 2, 3, 4      |
|Rotate Build Engine sprite       |5, 6            |
|Quit to DOS                      |Esc             |

## Command line arguments:
|Command line argument|Effect               |
|---------------------|---------------------|
|\<file\>             |Load custom map      |
|-noems               |Disable EMS          |

## Building:
1) Install [gcc-ia16](https://launchpad.net/%7Etkchia/+archive/ubuntu/build-ia16) (including [libi86](https://gitlab.com/tkchia/libi86)) on Ubuntu.

2) Run `bia16.sh`.

It's possible to build a 32-bit version of Build286 with [DJGPP](https://github.com/andrewwutw/build-djgpp) and [Watcom](https://github.com/open-watcom/open-watcom-v2.
First run `setenvdj.bat` once and then `bdj32.bat` for DJGPP, and `setenvwc.bat` followed by `bwc32.bat` for Watcom.
For debugging purposes, the Zone memory can be increased significantly this way.

It's also possible to build a 16-bit version with Watcom: Run `setenvwc.bat` followed by `bwc16.bat`.
